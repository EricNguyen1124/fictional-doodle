#!/usr/bin/env bash
set -euo pipefail
shopt -s nullglob

GLSLC=~/VulkanSDK/1.4.313.1/macOS/bin/glslc

for filename in shaders/*.{vert,frag}; do
  [ -f "$filename" ] || continue

  # Map by extension → output name
  case "$filename" in
    *.vert) out="vert.spv" ;;
    *.frag) out="frag.spv" ;;
    *) continue ;;
  esac

  "$GLSLC" "$filename" -o shaders/compiled/"$out"
done