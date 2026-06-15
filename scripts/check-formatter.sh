#!/usr/bin/env bash

echo "Checking code format..."

if find src include lib test \
\( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
-exec clang-format --dry-run --Werror {} +; then

    echo ""
    echo "Code formatted correctly."
    exit 0

else
    echo ""
    echo "Formatting not correct!"
    exit 1
fi
