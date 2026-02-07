mkdir -p /tmp/amp-win
cp build/amp-win.exe /tmp/amp-win
cp LICENSE /tmp/amp-win
cd /tmp
zip -r amp-win-${AMP_WIN_VERSION}-${AMP_ARCH}.zip amp-win
