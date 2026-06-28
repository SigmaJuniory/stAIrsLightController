#!/usr/bin/env zsh
# TODO: Add Wokwi to the CLI environment.
set -euo pipefail

cd "$(dirname "$0")/.."

pio test -e test-wokwi --without-uploading --without-testing

source ~/.zshrc

wokwi-cli \
  --elf .pio/build/test-wokwi/firmware.elf \
  --diagram-file diagram.json \
  --timeout 30000 \
  --expect-text OK \
  --fail-text FAIL \
  .
