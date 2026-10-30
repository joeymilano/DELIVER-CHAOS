#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
engine="${UNREAL_ENGINE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
"$engine/Engine/Build/BatchFiles/Mac/Build.sh" BadCatLabEditor Mac Development "$root/BadCatLab.uproject" -waitmutex
"$engine/Engine/Binaries/Mac/UnrealEditor-Cmd" "$root/BadCatLab.uproject" -run=pythonscript -script="$root/Tools/bootstrap.py" -unattended -nop4 -nullrhi -nosound
"$engine/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun -project="$root/BadCatLab.uproject" -noP4 -platform=Mac -clientconfig=Development -build -cook -stage -pak -package -archive -archivedirectory="$root/Builds" -unattended
