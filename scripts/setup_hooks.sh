#!/usr/bin/env bash

# Git pre-commit hook setup script
#
# This script installs a local git pre-commit hook that runs code formatting checks
# before each commit using ./scripts/check-formatter.sh.
#
# What it does:
# - Creates .git/hooks/pre-commit
# - Writes hook logic that executes clang-format check
# - Makes the hook executable
#
# When to run:
# - Run this script once after cloning the repository
# - Or whenever hooks are missing / reset (e.g. after git init, CI checkout, or fresh setup)
#
# Usage:
#   ./scripts/setup-hooks.sh
#
# Note:
# This hook is local to your machine and is not automatically shared via git.
# Each developer must run this setup script after cloning the repo.


set -e

echo " Setting up git pre-commit hook..."

# path do hooka
HOOK_PATH=".git/hooks/pre-commit"

# upewnij się że folder istnieje
mkdir -p .git/hooks

# tworzymy hook
cat > "$HOOK_PATH" << 'EOF'
#!/usr/bin/env bash

echo "Running formatter check..."

# always run from repo root
REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT"

./scripts/check-formatter.sh
EOF

# nadaj uprawnienia
chmod +x "$HOOK_PATH"

echo "Pre-commit hook installed successfully!"
