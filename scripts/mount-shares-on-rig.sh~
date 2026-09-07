#!/usr/bin/env bash
#
# mount-shares-on-rig.sh — mount this Mac's Scrivi SMB shares on the Linux rig.
#
# Run this FROM THE MAC. It drives oathkeeper over SSH and mounts:
#
#   //<mac>/ScriviLinux   -> /mnt/scrivi-net      (projects + appsupport + worlds subdir)
#   //<mac>/ScriviWorlds  -> /mnt/scrivi-worlds   (the "Scrivi Worlds" USB volume)
#
# Credentials come from a root-only file on the RIG (/etc/scrivi-share.creds),
# created by --setup-creds. The password is never written into this script, into
# the process table, or into a mount option visible to `ps`.
#
#   ./scripts/mount-shares-on-rig.sh --setup-share   # publish the Worlds share (sudo, this Mac)
#   ./scripts/mount-shares-on-rig.sh --setup-creds   # once, prompts for password
#   ./scripts/mount-shares-on-rig.sh                 # mount both shares
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
SHARE_WORLDS="${SCRIVI_SHARE_WORLDS:-ScriviWorlds}"

MOUNT_LINUX="${SCRIVI_MOUNT_LINUX:-/mnt/scrivi-net}"
MOUNT_WORLDS="${SCRIVI_MOUNT_WORLDS:-/mnt/scrivi-worlds}"

CREDS="/etc/scrivi-share.creds"

# The Mac-side directory published as the Worlds share. It lives on a REMOVABLE
# volume, and macOS drops a share point whose volume is absent — so this has to be
# re-checked (and sometimes re-created) every time the drive comes back.
WORLDS_VOLUME="${SCRIVI_WORLDS_VOLUME:-/Volumes/Scrivi Worlds}"

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
# name: the record for the Worlds volume is "Scrivi Worlds" while its smb_name is
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
# path contains a SPACE ("/Volumes/Scrivi Worlds") and whitespace-splitting
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

# Verify the ScriviWorlds share exists and points at the mounted volume,
# creating it if it does not. Requires sudo ON THIS MAC.
ensure_worlds_share() {
    say "Checking the '$SHARE_WORLDS' share on this Mac."

    if [ ! -d "$WORLDS_VOLUME" ]; then
        die "the Worlds volume is not mounted at:  $WORLDS_VOLUME
    It is a REMOVABLE drive — plug it in / mount it, then re-run.
    (Override the path with SCRIVI_WORLDS_VOLUME=...)"
    fi
    say "  volume present: $WORLDS_VOLUME"

    if share_exists "$SHARE_WORLDS"; then
        # It exists — make sure it still points where we think it does. A share
        # point that survived a remount can be stale.
        local actual
        actual="$(share_path "$SHARE_WORLDS")"
        if [ -n "$actual" ] && [ "$actual" != "$WORLDS_VOLUME" ]; then
            warn "  share '$SHARE_WORLDS' exists but points at: $actual"
            warn "  expected: $WORLDS_VOLUME"
            warn "  Remove and recreate it with:  sudo sharing -r $SHARE_WORLDS"
            return 1
        fi
        say "  share '$SHARE_WORLDS' already published -> $WORLDS_VOLUME"
        return 0
    fi

    say "  not published — creating it (sudo on THIS MAC will prompt)."
    # -s 001 / -g 000: SMB on, guest OFF. We authenticate with the credentials
    # file on the rig, so guest access is unnecessary exposure for real writing work.
    sudo sharing -a "$WORLDS_VOLUME" -S "$SHARE_WORLDS" -n "$SHARE_WORLDS" -s 001 -g 000 \
        || die "could not create the '$SHARE_WORLDS' share point"

    if share_exists "$SHARE_WORLDS"; then
        say "  created '$SHARE_WORLDS' -> $WORLDS_VOLUME"
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
    local share="$1" mp="$2"

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
    if [ "$mp" = "$MOUNT_WORLDS" ]; then
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
    ensure_worlds_share
    require_creds

    local failed=0
    mount_share "$SHARE_LINUX"  "$MOUNT_LINUX"   || failed=1
    mount_share "$SHARE_WORLDS" "$MOUNT_WORLDS"  || failed=1

    echo
    do_status

    if [ "$failed" -ne 0 ]; then
        echo
        warn "At least one share did not mount.  Most likely causes, in order:"
        warn "  1. The share is not published on the Mac.  Check with:  sharing -l"
        warn "     A share point whose VOLUME WAS ABSENT can be dropped by macOS and"
        warn "     needs re-adding after the drive comes back."
        warn "  2. The '$SHARE_WORLDS' share lives on the removable 'Scrivi Worlds'"
        warn "     volume — confirm it is mounted here:  ls /Volumes/"
        warn "  3. Wrong password in $CREDS — re-run with --setup-creds."
        warn "  4. 'mount error(16): Device or resource busy' means a STALE mount still"
        warn "     holds the path. This script clears those automatically; if one"
        warn "     survives, clear it by hand:"
        warn "       ssh -t $RIG \"sudo umount -l $MOUNT_WORLDS\""
        return 1
    fi
}

# --- status / unmount --------------------------------------------------------

do_status() {
    say "Mounted on $RIG:"
    # STALE is reported separately from "not mounted": they look identical to
    # `mountpoint` but need different fixes, and only STALE causes EBUSY.
    rig "for mp in '$MOUNT_LINUX' '$MOUNT_WORLDS'; do
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
    for mp in "$MOUNT_LINUX" "$MOUNT_WORLDS"; do
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

case "${1:-}" in
    --setup-creds) setup_creds ;;
    --setup-share) ensure_worlds_share ;;
    --status)      preflight; do_status ;;
    --unmount|-u)  do_unmount ;;
    --help|-h)
        sed -n '2,21p' "$0" | sed 's/^# \{0,1\}//'
        ;;
    "")            do_mount ;;
    *)             die "unknown option: $1  (try --help)" ;;
esac
