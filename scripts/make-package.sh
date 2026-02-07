#!/bin/bash
# Clean up any stuff from previous builds
rm /tmp/amp-win-${AMP_WIN_VERSION}-${AMP_ARCH}.zip
rm -rf /tmp/amp-win
mkdir -p /tmp/amp-win
# Install
cp build/amp-win.exe /tmp/amp-win/Ampersand.exe
cp LICENSE /tmp/amp-win
# Make the package
cd /tmp
zip -r amp-win-${AMP_WIN_VERSION}-${AMP_ARCH}.zip amp-win
