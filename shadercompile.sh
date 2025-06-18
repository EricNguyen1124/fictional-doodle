#!/usr/bin/env bash
set -euo pipefail
shopt -s nullglob        # prevents literal patterns when no files match

for filename in shaders/*.{vert,frag,comp}.hlsl; do
    [ -f "$filename" ] || continue         # extra safety

    base="${filename%.hlsl}"               # strip the .hlsl once
    shadercross "$filename" -o "${base}.dxil"
    shadercross "$filename" -o "${base}.msl"
done