#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
source_video="${1:?Pass the user-provided OIIA video path}"
mkdir -p "$root/ArtSource/MemesLocal"
ffmpeg -v error -ss 2.5 -i "$source_video" -vf 'fps=12,crop=512:512:104:384,scale=256:256,colorkey=0x00ff00:0.3:0.1,format=rgba,tile=8x4' -frames:v 1 -y "$root/ArtSource/MemesLocal/OiiaAtlas.png"
ffmpeg -v error -ss 2.5 -t 2.667 -i "$source_video" -filter_complex '[0:v]fps=12,crop=512:512:104:384,scale=320:320,colorkey=0x00ff00:0.3:0.1,split[a][b];[a]palettegen=reserve_transparent=1[p];[b][p]paletteuse=alpha_threshold=128' -loop 0 -y "$root/ArtSource/MemesLocal/oiia-easter-egg.gif"
