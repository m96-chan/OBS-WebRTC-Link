#!/bin/bash
# Version bump script for OBS-WebRTC-Link
# Usage: ./scripts/bump-version.sh [major|minor|patch]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERSION_FILE="$PROJECT_ROOT/VERSION"
CHANGELOG_FILE="$PROJECT_ROOT/CHANGELOG.md"

# Check if VERSION file exists
if [ ! -f "$VERSION_FILE" ]; then
    echo -e "${RED}Error: VERSION file not found at $VERSION_FILE${NC}"
    exit 1
fi

# Read current version
CURRENT_VERSION=$(cat "$VERSION_FILE" | tr -d '[:space:]')
echo -e "${GREEN}Current version: $CURRENT_VERSION${NC}"

# Parse version components
IFS='.' read -r -a VERSION_PARTS <<< "$CURRENT_VERSION"
MAJOR="${VERSION_PARTS[0]}"
MINOR="${VERSION_PARTS[1]}"
PATCH="${VERSION_PARTS[2]}"

# Determine bump type
BUMP_TYPE="${1:-patch}"

case "$BUMP_TYPE" in
    major)
        MAJOR=$((MAJOR + 1))
        MINOR=0
        PATCH=0
        ;;
    minor)
        MINOR=$((MINOR + 1))
        PATCH=0
        ;;
    patch)
        PATCH=$((PATCH + 1))
        ;;
    *)
        echo -e "${RED}Error: Invalid bump type. Use 'major', 'minor', or 'patch'${NC}"
        echo "Usage: $0 [major|minor|patch]"
        exit 1
        ;;
esac

NEW_VERSION="$MAJOR.$MINOR.$PATCH"
echo -e "${GREEN}New version: $NEW_VERSION${NC}"

# Confirm with user
read -p "Bump version from $CURRENT_VERSION to $NEW_VERSION? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Version bump cancelled${NC}"
    exit 0
fi

# Update VERSION file
echo "$NEW_VERSION" > "$VERSION_FILE"
echo -e "${GREEN}Updated VERSION file${NC}"

# Update CHANGELOG.md
TODAY=$(date +%Y-%m-%d)
if [ -f "$CHANGELOG_FILE" ]; then
    # Create backup
    cp "$CHANGELOG_FILE" "$CHANGELOG_FILE.bak"

    # Replace [Unreleased] with new version
    sed -i.tmp "s/## \[Unreleased\]/## [Unreleased]\n\n## [$NEW_VERSION] - $TODAY/" "$CHANGELOG_FILE"

    # Update version links at bottom
    sed -i.tmp "s|\[Unreleased\]:.*|\[Unreleased\]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v$NEW_VERSION...HEAD\n[$NEW_VERSION]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v$CURRENT_VERSION...v$NEW_VERSION|" "$CHANGELOG_FILE"

    # Clean up temp files
    rm -f "$CHANGELOG_FILE.tmp" "$CHANGELOG_FILE.bak"

    echo -e "${GREEN}Updated CHANGELOG.md${NC}"
else
    echo -e "${YELLOW}Warning: CHANGELOG.md not found${NC}"
fi

# Git operations
echo -e "${YELLOW}Creating git commit and tag...${NC}"

# Check if there are changes to commit
if ! git diff --quiet VERSION CHANGELOG.md 2>/dev/null; then
    git add VERSION CHANGELOG.md
    git commit -m "chore: bump version to $NEW_VERSION

- Updated VERSION file
- Updated CHANGELOG.md with release date

[skip ci]"

    # Create git tag
    git tag -a "v$NEW_VERSION" -m "Release version $NEW_VERSION"

    echo -e "${GREEN}Created commit and tag v$NEW_VERSION${NC}"
    echo -e "${YELLOW}To push changes, run:${NC}"
    echo "  git push origin main"
    echo "  git push origin v$NEW_VERSION"
else
    echo -e "${YELLOW}No changes to commit${NC}"
fi

echo -e "${GREEN}Version bump complete!${NC}"
