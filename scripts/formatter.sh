#!/usr/bin/env bash

# -----------------------------------------------------------------------------
# Code formatter (auto-fix)
#
# This script automatically formats source code using clang-format.
# It applies formatting changes directly to files (in-place).
#
# What it does:
# - Searches for all .cpp, .h, and .hpp files in:
#     src/, include/, lib/, test/
# - Runs clang-format in "edit mode" (-i), which modifies files directly
# - Uses formatting rules defined in .clang-format file in the project root
#
# Behavior:
# - All matching files are automatically reformatted
# - No output is required unless an error occurs
# - Exit code 0 means formatting completed successfully
#
# IMPORTANT:
# - This script MODIFIES files automatically
# - Use before commit or as part of pre-commit workflow
#
# Typical usage:
#   - manual formatting before commit
#   - CI helper (optional)
#   - developer tool for fixing style issues
# -----------------------------------------------------------------------------


set -e

echo "Formatting code with clang-format..."

find src include lib test \
\( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
-exec clang-format -i {} +

echo "Formatting done."
