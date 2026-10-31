#!/bin/bash
set -euo pipefail
# Keep this project on the verified SDK when other projects switch Xcode.
export DEVELOPER_DIR="${BAD_CAT_DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
root="$(cd "$(dirname "$0")/.." && pwd)"
configuration="${BAD_CAT_CONFIGURATION:-Shipping}"
engine="${UNREAL_ENGINE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
python3 "$root/Tools/check_collaboration_assets.py"
"$engine/Engine/Build/BatchFiles/Mac/Build.sh" BadCatLabEditor Mac Development "$root/BadCatLab.uproject" -waitmutex
"$engine/Engine/Binaries/Mac/UnrealEditor-Cmd" "$root/BadCatLab.uproject" -run=pythonscript -script="$root/Tools/bootstrap.py" -unattended -nop4 -nullrhi -nosound
"$engine/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun -project="$root/BadCatLab.uproject" -noP4 -platform=Mac -clientconfig="$configuration" -build -cook -stage -pak -package -archive -archivedirectory="$root/Builds" -unattended

# UAT retains the configuration suffix for Shipping. Never sign/export a stale
# Development bundle left at the old unsuffixed path.
packaged="$root/Builds/Mac/BadCatLab.app"
if [[ "$configuration" == "Shipping" ]]; then
  packaged="$root/Builds/Mac/BadCatLab-Mac-Shipping.app"
fi
test -f "$packaged/Contents/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :NSHighResolutionCapable true' "$packaged/Contents/Info.plist"
/usr/bin/codesign --force --deep --sign - "$packaged"
canonical="$root/Builds/Mac/BadCatLab.app"
if [[ "$packaged" != "$canonical" ]]; then
  if [[ -e "$canonical" ]]; then
    mkdir -p "$root/Builds/Previous"
    mv "$canonical" "$root/Builds/Previous/BadCatLab-$(date +%Y%m%d-%H%M%S).app"
  fi
  /usr/bin/ditto "$packaged" "$canonical"
fi
echo "Mac game: $canonical ($configuration)"
