#!/usr/bin/env bash

set -e

echo "Formatting code with clang-format..."

find src include lib test \
\( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
-exec clang-format -i {} +

echo "Formatting done."
