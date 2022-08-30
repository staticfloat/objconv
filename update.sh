#!/usr/bin/env bash

set -euo pipefail

check_tool() {
    if [[ -z "$(which "${1}")" ]]; then
        echo "Must install '${1}'!"
        exit 1
    fi
}
check_tool "unzip"
check_tool "dos2unix"
check_tool "perl"

# Save our repo root
REPO_ROOT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Download latest `objconv.zip`
T=$(mktemp -d)
trap "rm -rf ${T}" EXIT
curl -L "https://www.agner.org/optimize/objconv.zip" -o "${T}/objconv.zip"
mkdir "${T}/extracted"
cd "${T}/extracted"
unzip "${T}/objconv.zip"

# Extract inner "source.zip" to "${REPO_ROOT}/src"
cd "${REPO_ROOT}/src"
rm -f *
unzip "${T}/extracted/source.zip"

# Clean up the source a bit
dos2unix *
perl -pi -e 's/ +$//' *

# Copy over a new changelog.txt
cd "${REPO_ROOT}"
cp "${T}/extracted/changelog.txt" "changelog.txt"
dos2unix "changelog.txt"

# Commit the new version
git add .
git commit -am "$(head -1 changelog.txt)"
