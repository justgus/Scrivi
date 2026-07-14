#pragma once

#include <QString>

// AppSupport — resolves the Linux app-support root (SP-059 / T-0223).
//
// The Linux counterpart to macOS Application Support. Per the SP-059 Step-0
// convention (see platforms/linux/README.md, "App-support root"):
//
//   $XDG_DATA_HOME/Scrivi   if $XDG_DATA_HOME is set and non-empty
//   $HOME/.local/share/Scrivi   otherwise
//
// The directory is created (mkdir -p) on first resolution. The returned absolute
// path is passed into the existing scrivi_* C-ABI `appSupportRoot` parameters —
// no ScriviCore change. It retires EP-020's throwaway /tmp/scrivi-linux-appsupport,
// under which identity was recreated on every run.
namespace scrivi::linux_app {

// Resolves the app-support root, creating the directory if it does not exist.
// Returns the absolute path (no trailing slash). Reads $XDG_DATA_HOME and $HOME
// from the environment; if $HOME is also unset (unusual), falls back to the Qt
// standard writable data location so a path is always returned.
QString appSupportRoot();

} // namespace scrivi::linux_app
