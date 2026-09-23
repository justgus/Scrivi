#!/usr/bin/env bash
#
# mount-shares-on-rig.sh — mount this Mac's Scrivi SMB shares on the Linux rig.
#
# Run this FROM THE MAC. It drives oathkeeper over SSH and mounts:
#
#   //<mac>/ScriviLinux   -> /mnt/scrivi-net      (projects + appsupport + worlds subdir)
#   //<mac>/ScriviWorlds  -> /mnt/scrivi-worlds   (the "Scrivi-Worlds" USB volume)
#   //<mac>/ScriviOther   -> /mnt/scrivi-other    (the "SCRIVI-OTHE" USB volume)
#
# ⚠️ THE REMOVABLE VOLUMES ARE A LIST AND MOUNT INDEPENDENTLY. Either or both can
# be mounted at once; a drive that is NOT ATTACHED is skipped with a note, never
# an error, so one unplugged volume cannot block the others.
#
# Credentials come from a root-only file on the RIG (/etc/scrivi-share.creds),
# created by --setup-creds. The password is never written into this script, into
# the process table, or into a mount option visible to `ps`.
#
#   ./scripts/mount-shares-on-rig.sh --setup-share   # publish the Worlds share (sudo, this Mac)
#   ./scripts/mount-shares-on-rig.sh --setup-creds   # once, prompts for password
#   ./scripts/mount-shares-on-rig.sh                 # mount both shares
#   ./scripts/mount-shares-on-rig.sh --no-worlds     # mount ONLY ScriviLinux
#   ./scripts/mount-shares-on-rig.sh --only ScriviOther   # ScriviLinux + ONE removable
#
# ⚠️ --no-worlds (or SCRIVI_SKIP_WORLDS=1) exists because the Worlds share lives
# on a REMOVABLE volume, and a scenario that does not need it should not be
# blocked by its absence. T-0477's S2 (share killed at the source) serves its
# world from ~/ScriviLinux/worlds/ on LOCAL disk precisely so it can run with the
# drive unplugged — ⚠️ and deliberately keeps real writing work OUT of the blast
# radius of a test that kills a share mid-use.
#   ./scripts/mount-shares-on-rig.sh --status        # what is mounted right now
#   ./scripts/mount-shares-on-rig.sh --unmount       # unmount both
#
# Host defaults match deploy-to-rig.sh: SCRIVI_RIG=oathkeeper.
#
set -euo pipefail

RIG="${SCRIVI_RIG:-oathkeeper}"
MAC_HOST="${SCRIVI_MAC_HOST:-$(scutil --get LocalHostName 2>/dev/null || hostname -s)}.local"
SMB_USER="${SCRIVI_SMB_USER:-$(id -un)}"

SHARE_LINUX="${SCRIVI_SHARE_LINUX:-ScriviLinux}"
MOUNT_LINUX="${SCRIVI_MOUNT_LINUX:-/mnt/scrivi-net}"

CREDS="/etc/scrivi-share.creds"

# ⚠️ Skip ALL removable volumes. Set by --no-worlds, or exported.
SKIP_WORLDS="${SCRIVI_SKIP_WORLDS:-0}"

# --- REMOVABLE VOLUMES -------------------------------------------------------
#
# ⚠️ THERE IS MORE THAN ONE, AND THEY MOUNT INDEPENDENTLY (2026-09-22).
#
# This was a single `Scrivi-Worlds` volume until a live pass needed
# `SCRIVI-OTHE` — the drive holding the real test projects
# (the-stairs-of-tintagael, Eskandar.scrivworld, the-lone-golem). Reusing the
# single slot would have silently displaced the Worlds mount, so the volumes are
# a LIST and any subset can be mounted at once.
#
# Each entry is:   <macOS volume path>|<SMB share name>|<rig mount point>
#
# ⚠️ These live on REMOVABLE drives, and macOS DROPS a share point whose volume
# is absent — so each has to be re-checked (and sometimes re-created) every time
# its drive comes back. That is what ensure_share() does.
#
# ⚠️ A volume that is not attached is SKIPPED WITH A NOTE, not an error: mounting
# one drive must not be blocked by another being unplugged. That is the same
# lesson --no-worlds encoded for a single volume.
REMOVABLES=(
    "${SCRIVI_WORLDS_VOLUME:-/Volumes/Scrivi-Worlds}|${SCRIVI_SHARE_WORLDS:-ScriviWorlds}|${SCRIVI_MOUNT_WORLDS:-/mnt/scrivi-worlds}"
    "${SCRIVI_OTHER_VOLUME:-/Volumes/SCRIVI-OTHE}|${SCRIVI_SHARE_OTHER:-ScriviOther}|${SCRIVI_MOUNT_OTHER:-/mnt/scrivi-other}"
)

# Restrict to ONE removable by name (--only <share>), for a scenario that must
# not have the other drive in its blast radius.
ONLY_SHARE="${SCRIVI_ONLY_SHARE:-}"

# All rig mount points this script manages, for --status and --unmount.
all_mounts() {
    printf '%s\n' "$MOUNT_LINUX"
    local entry
    for entry in "${REMOVABLES[@]}"; do printf '%s\n' "${entry##*|}"; done
}

SSH_OPTS=(-o ConnectTimeout=10 -o ServerAliveInterval=5 -o ServerAliveCountMax=3)

say()  { printf '==> %s\n' "$*"; }
warn() { printf '!!  %s\n' "$*" >&2; }
die()  { printf '!!  %s\n' "$*" >&2; exit 1; }

rig() { ssh "${SSH_OPTS[@]}" "$RIG" "$@"; }

# sudo on the rig requires a password and therefore a TTY. A plain `ssh host cmd`
# has no TTY, which fails as:  "sudo: a terminal is required to authenticate".
# -t forces one so sudo can prompt you here.
# -tt (not -t) FORCES a pty even when this script's own stdin is not a terminal
# (a pipeline, a CI step). With plain -t, ssh declines and sudo fails with
# "a terminal is required to authenticate".
rig_sudo() { ssh -tt "${SSH_OPTS[@]}" "$RIG" "$@"; }

# --- preflight ---------------------------------------------------------------

preflight() {
    say "Rig:    $RIG"
    say "Mac:    $MAC_HOST  (SMB user: $SMB_USER)"

    # The box sleeps. A failed connect means ASLEEP, not broken.
    if ! rig true 2>/dev/null; then
        die "cannot reach $RIG over SSH.
    The rig SLEEPS — a refused connection usually means asleep, not down.
    Wake it (physically, or wake-on-LAN) and re-run."
    fi

    rig 'command -v mount.cifs >/dev/null 2>&1' \
        || die "cifs-utils is not installed on $RIG.  Fix:  ssh $RIG 'sudo apt install -y cifs-utils'"
}

# --- Mac-side share points ---------------------------------------------------

# share_exists <smb-share-name>  — is it published on THIS Mac right now?
# Matches the SMB name (the name the rig mounts), which is NOT always the record
# name: the record for the Worlds volume is "Scrivi-Worlds" while its smb_name is
# "ScriviWorlds".
share_exists() {
    # NOT `| grep -q`: grep exits on first match, `sharing` takes SIGPIPE, and
    # `set -o pipefail` then reports rc=141 for a SUCCESSFUL match — which reads as
    # "share missing" and would create a duplicate share point.
    # Capture first, match second, so no signal can reach the producer.
    local listing
    listing="$(sharing -l -f json 2>/dev/null)" || return 1
    case "$listing" in
        *"\"smb_name\" : \"$1\""*) return 0 ;;
    esac
    # Tolerate spacing differences around the colon.
    printf '%s' "$listing" | tr -d ' ' | grep -q "\"smb_name\":\"$1\""
}

# share_path <smb-share-name>  — the directory behind that SMB name, or empty.
# Read from the JSON, because the plain listing cannot be parsed safely: the real
# path contains a SPACE ("/Volumes/Scrivi-Worlds") and whitespace-splitting
# truncates it to "/Volumes/Scrivi".
share_path() {
    sharing -l -f json 2>/dev/null | python3 -c '
import json, sys
try:
    shares = json.load(sys.stdin)
except Exception:
    sys.exit(0)
want = sys.argv[1]
for rec in shares.values():
    if isinstance(rec, dict) and rec.get("smb_name") == want:
        print(rec.get("path", ""))
        break
' "$1" 2>/dev/null
}

# share_record <smb-share-name>  — the RECORD name for an SMB share name.
#
# ⚠️ THESE ARE DIFFERENT NAMES AND `sharing -r` TAKES THE RECORD NAME.
# `sharing -a <path> -S <smb-name>` sets the SMB name, but the RECORD is named
# after the directory — so a volume renamed on disk leaves a record still called
# by the OLD name while the SMB name is unchanged.
#
# ⚠️ Observed 2026-09-07: after "Scrivi Worlds" was renamed "Scrivi-Worlds", the
# record was still `Scrivi Worlds` and `sudo sharing -r ScriviWorlds` failed with
# "unable to find share point record" — advice this script itself had printed.
share_record() {
    /usr/sbin/sharing -l 2>/dev/null | awk -v want="$1" '
        /^name:/        { rec = $0; sub(/^name:[ \t]*/, "", rec) }
        /^[ \t]*name:/  { smb = $0; sub(/^[ \t]*name:[ \t]*/, "", smb)
                          if (smb == want) { print rec; exit } }
    '
}

# Verify the ScriviWorlds share exists and points at the mounted volume,
# creating it if it does not. Requires sudo ON THIS MAC.
# ensure_share <volume-path> <smb-share-name>
#
# ⚠️ Returns 2 when the VOLUME IS ABSENT — deliberately NOT an error. A drive that
# is unplugged must not stop the others mounting; the caller reports and moves on.
ensure_share() {
    local vol="$1" share="$2"
    say "Checking the '$share' share on this Mac."

    if [ ! -d "$vol" ]; then
        warn "  volume not attached: $vol  — skipping '$share'"
        return 2
    fi
    say "  volume present: $vol"

    if share_exists "$share"; then
        # It exists — make sure it still points where we think it does. A share
        # point that survived a remount can be stale.
        local actual
        actual="$(share_path "$share")"
        if [ -n "$actual" ] && [ "$actual" != "$vol" ]; then
            warn "  share '$share' exists but points at: $actual"
            warn "  expected: $vol"
            # ⚠️ Quote it: the record name is the OLD DIRECTORY name and usually
            # contains a space. Naming the SMB share here is what made this
            # advice fail on 2026-09-07.
            local rec
            rec="$(share_record "$share")"
            if [ -n "$rec" ]; then
                warn "  Remove and recreate it with:"
                warn "      sudo sharing -r \"$rec\""
                warn "      $0 --setup-share"
            else
                warn "  Remove it by RECORD name (see: sharing -l), then --setup-share"
            fi
            return 1
        fi
        say "  share '$share' already published -> $vol"
        return 0
    fi

    say "  not published — creating it (sudo on THIS MAC will prompt)."
    # -s 001 / -g 000: SMB on, guest OFF. We authenticate with the credentials
    # file on the rig, so guest access is unnecessary exposure for real writing work.
    sudo sharing -a "$vol" -S "$share" -n "$share" -s 001 -g 000 \
        || die "could not create the '$share' share point"

    if share_exists "$share"; then
        say "  created '$share' -> $vol"
    else
        die "created the share but it does not appear in \`sharing -l\` — check System Settings ▸ Sharing"
    fi
}

# --- credentials -------------------------------------------------------------

setup_creds() {
    preflight
    say "Creating $CREDS on $RIG for SMB user '$SMB_USER'."
    say "This is the password for the macOS account that owns the shares."

    local pw pw2
    read -r -s -p "SMB password for $SMB_USER@$MAC_HOST: " pw; echo
    read -r -s -p "Repeat: " pw2; echo
    [ -n "$pw" ]     || die "empty password"
    [ "$pw" = "$pw2" ] || die "passwords do not match"

    # Two-step, because the password cannot share stdin with an interactive sudo:
    #   (a) stream the credentials to a private temp file on the rig  — no sudo, stdin free
    #   (b) install that file into place with a TTY sudo              — no stdin needed
    # The SMB password travels over the ssh channel on stdin only, never in argv,
    # so it cannot appear in `ps` on either machine.
    local stage="/tmp/.scrivi-creds.$$"

    printf 'username=%s\npassword=%s\n' "$SMB_USER" "$pw" \
        | rig "umask 077 && cat > '$stage'"

    unset pw pw2

    say "Installing $CREDS (sudo on $RIG will prompt for your LINUX password)."
    rig_sudo "sudo install -m 600 -o root -g root '$stage' $CREDS && rm -f '$stage'" \
        || { rig "rm -f '$stage'" 2>/dev/null; die "failed to install $CREDS"; }
    say "Wrote $CREDS (root-only, mode 600)."
}

require_creds() {
    # `test -r` as a normal user is false for a 0600 root file, so check existence
    # without sudo — that avoids a TTY prompt just to find out whether to continue.
    rig "[ -e $CREDS ]" 2>/dev/null \
        || die "no credentials file on $RIG.  Run:  $0 --setup-creds"
}

# --- mounting ----------------------------------------------------------------

# is_stale <mount-point> — a ZOMBIE mount: still in the kernel's mount table, but
# the SMB session behind it is dead. Happens whenever the USB volume is pulled.
# The tell is a contradiction: `mount` lists it, `mountpoint -q` says no, and any
# stat returns ESTALE. A fresh mount over it fails with EBUSY (mount error 16).
is_stale() {
    local mp="$1"
    # Listed in the mount table...
    rig "grep -q ' $mp ' /proc/mounts" 2>/dev/null || return 1
    # ...but not answering as a mountpoint: that is the zombie signature.
    rig "mountpoint -q '$mp'" 2>/dev/null && return 1
    return 0
}

# Clear a zombie so the mount point can be reused. Lazy detach first (works even
# while something holds a dead handle), then a forced unmount as a fallback.
clear_stale() {
    local mp="$1"
    warn "STALE mount at $mp — the volume was pulled while mounted."
    warn "Clearing it (sudo on $RIG will prompt)."
    rig_sudo "sudo umount -l '$mp' 2>/dev/null; sudo umount -f '$mp' 2>/dev/null; true" >/dev/null 2>&1
    if is_stale "$mp"; then
        warn "could not clear the stale mount at $mp"
        return 1
    fi
    say "  cleared $mp"
    return 0
}

# mount_share <share-name> <mount-point>
mount_share() {
    local share="$1" mp="$2"   # $3 = "removable" for a pulled-drive volume

    if rig "mountpoint -q '$mp'" 2>/dev/null; then
        say "already mounted: $mp"
        return 0
    fi

    # Not a live mountpoint — but it may still be a zombie holding the path,
    # which would make the mount below fail with EBUSY. Clear it first.
    if is_stale "$mp"; then
        clear_stale "$mp" || return 1
    fi

    # uid/gid map the share to the rig's own user so the app can write as itself.
    # vers=3.0 is what macOS SMB negotiates; soft avoids an unkillable hang if the
    # Mac sleeps or the USB volume is pulled mid-session.
    #
    # serverino + cache=none are DELIBERATE, and were earned:
    #
    #   Observed 2026-09-01. The Worlds volume was pulled on the Mac and replugged.
    #   /mnt/scrivi-worlds then served a listing of a DIFFERENT share (ScriviLinux)
    #   — byte-identical entries, same sizes, same timestamps — while the real
    #   content (Eskandar.scrivworld) returned ENOENT and the phantom entries
    #   returned EBADF. A cached listing with nothing behind it.
    #
    #   noserverino makes the client invent inode numbers instead of using the
    #   server's, so cached entries from one share can be presented under another.
    #   cache=strict (the default) then keeps serving them after the volume is gone.
    #
    # For a REMOVABLE volume holding real writing work, a stale listing is worse
    # than an error: the app cannot tell "world is missing" from "world is fine".
    local opts="credentials=$CREDS,uid=\$(id -u),gid=\$(id -g),file_mode=0755,dir_mode=0755,vers=3.0,soft,serverino,actimeo=1"

    # The removable share gets no caching at all, so a pulled drive fails loudly
    # and immediately rather than being papered over by the client cache.
    # ⚠️ EVERY REMOVABLE gets cache=none, not just one of them. A pulled drive must
    # fail loudly and immediately rather than be papered over by the client cache
    # (see the 2026-09-01 phantom-listing note above). `$3` is set by the caller for
    # a removable share and empty for ScriviLinux, which lives on local disk.
    if [ "${3:-}" = "removable" ]; then
        opts="$opts,cache=none"
    fi

    say "mounting //$MAC_HOST/$share -> $mp"
    if rig_sudo "sudo mkdir -p '$mp' && sudo mount -t cifs '//$MAC_HOST/$share' '$mp' -o \"$opts\"" 2>&1; then
        say "  ok"
        return 0
    fi

    warn "failed to mount //$MAC_HOST/$share"
    return 1
}

do_mount() {
    preflight
    require_creds

    local failed=0

    # ⚠️ ScriviLinux is mounted FIRST and unconditionally. It lives on LOCAL disk,
    # so it must never be blocked by the removable Worlds volume being absent —
    # which is exactly what happened before --no-worlds existed.
    mount_share "$SHARE_LINUX"  "$MOUNT_LINUX"   || failed=1

    if [ "$SKIP_WORLDS" = "1" ]; then
        say "skipping ALL removable volumes (--no-worlds)"
    else
        local entry vol share mp rc
        for entry in "${REMOVABLES[@]}"; do
            vol="${entry%%|*}"
            mp="${entry##*|}"
            share="${entry#*|}"; share="${share%%|*}"

            if [ -n "$ONLY_SHARE" ] && [ "$share" != "$ONLY_SHARE" ]; then
                say "skipping '$share' (--only $ONLY_SHARE)"
                continue
            fi

            # ⚠️ rc=2 means the DRIVE IS NOT ATTACHED. That is a skip, not a failure:
            # one unplugged volume must not stop the others mounting.
            ensure_share "$vol" "$share"; rc=$?
            [ "$rc" -eq 2 ] && continue
            [ "$rc" -ne 0 ] && { failed=1; continue; }

            mount_share "$share" "$mp" removable || failed=1
        done
    fi

    echo
    do_status

    if [ "$failed" -ne 0 ]; then
        echo
        warn "At least one share did not mount.  Most likely causes, in order:"
        warn "  1. The share is not published on the Mac.  Check with:  sharing -l"
        warn "     A share point whose VOLUME WAS ABSENT can be dropped by macOS and"
        warn "     needs re-adding after the drive comes back."
        warn "  2. A removable share's volume may be unplugged — confirm with:  ls /Volumes/"
        warn "     A volume that is ABSENT is skipped with a note, not an error."
        warn "  3. Wrong password in $CREDS — re-run with --setup-creds."
        warn "  4. 'mount error(16): Device or resource busy' means a STALE mount still"
        warn "     holds the path. This script clears those automatically; if one"
        warn "     survives, clear it by hand:"
        warn "       ssh -t $RIG \"sudo umount -l <mount-point>\""
        return 1
    fi
}

# --- status / unmount --------------------------------------------------------

do_status() {
    say "Mounted on $RIG:"
    # STALE is reported separately from "not mounted": they look identical to
    # `mountpoint` but need different fixes, and only STALE causes EBUSY.
    rig "for mp in $(all_mounts | sed "s/^/'/;s/$/'/" | tr '\n' ' '); do
            if mountpoint -q \"\$mp\" 2>/dev/null; then
                printf '    %-22s MOUNTED   (%s entries)\n' \"\$mp\" \"\$(ls -1 \"\$mp\" 2>/dev/null | wc -l | tr -d ' ')\"
            elif grep -q \" \$mp \" /proc/mounts 2>/dev/null; then
                printf '    %-22s STALE     (volume pulled; will be cleared on next mount)\n' \"\$mp\"
            else
                printf '    %-22s not mounted\n' \"\$mp\"
            fi
         done"
}

do_unmount() {
    preflight
    for mp in $(all_mounts); do
        if rig "mountpoint -q '$mp'" 2>/dev/null; then
            say "unmounting $mp"
            # lazy unmount as a fallback: a soft cifs mount can still be busy if
            # the app has a file open on it.
            rig_sudo "sudo umount '$mp' 2>/dev/null || sudo umount -l '$mp'" \
                && say "  ok" || warn "  could not unmount $mp"
        else
            say "not mounted: $mp"
        fi
    done
}

# --- main --------------------------------------------------------------------

# ensure_all_shares — publish every ATTACHED removable volume's share.
ensure_all_shares() {
    local entry vol share
    for entry in "${REMOVABLES[@]}"; do
        vol="${entry%%|*}"
        share="${entry#*|}"; share="${share%%|*}"
        [ -n "$ONLY_SHARE" ] && [ "$share" != "$ONLY_SHARE" ] && continue
        ensure_share "$vol" "$share" || true
    done
}

case "${1:-}" in
    --setup-creds) setup_creds ;;
    --setup-share) ensure_all_shares ;;
    --status)      preflight; do_status ;;
    --unmount|-u)  do_unmount ;;
    --no-worlds)   SKIP_WORLDS=1; do_mount ;;
    --only)        [ -n "${2:-}" ] || die "--only needs a share name (see --help)"
                   ONLY_SHARE="$2"; do_mount ;;
    --help|-h)
        sed -n '2,29p' "$0" | sed 's/^# \{0,1\}//'
        ;;
    "")            do_mount ;;
    *)             die "unknown option: $1  (try --help)" ;;
esac
