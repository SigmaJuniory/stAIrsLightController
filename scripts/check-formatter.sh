#!/usr/bin/env bash

# -----------------------------------------------------------------------------
# Code format checker (pre-commit / CI helper)
#
# This script checks whether the source code is properly formatted
# according to the rules defined in .clang-format.
#
# What it does:
# - Searches for all .cpp, .h, and .hpp files in:
#     src/, include/, lib/, test/
# - Runs clang-format in "dry-run" mode (no changes are made)
# - Uses --Werror to fail if any formatting issues are found
#
# Behavior:
# - If all files are correctly formatted:
#     → prints "Code formatted correctly."
#     → exits with code 0 (success)
#
# - If any file is NOT correctly formatted:
#     → prints "Formatting not correct!"
#     → exits with code 1 (failure)
#
# Typical usage:
#   - pre-commit git hook
#   - CI pipeline check
#   - manual verification before commit
# -----------------------------------------------------------------------------


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
