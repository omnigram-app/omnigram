#!/bin/bash
set -e

echo "=== Building OmniGram AppImage ==="

cd out/Debug
strip Telegram || true

mkdir -p AppDir/usr/bin AppDir/usr/share/icons/hicolor/512x512/apps
cp Telegram AppDir/usr/bin/Telegram
cp Updater AppDir/usr/bin/Updater || true
cp ../../lib/xdg/org.telegram.desktop.desktop AppDir/org.telegram.desktop.desktop
cp ../../Telegram/Resources/art/icon512.png AppDir/org.telegram.desktop.png
cp ../../Telegram/Resources/art/icon512.png AppDir/usr/share/icons/hicolor/512x512/apps/org.telegram.desktop.png

# Create AppRun launcher
cat > AppDir/AppRun << 'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/Telegram" "$@"
EOF

chmod +x AppDir/AppRun

# Download appimagetool
curl -sLO https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage --appimage-extract
./squashfs-root/AppRun --no-appstream AppDir OmniGram-x86_64.AppImage || true

mkdir -p artifact
mv {Telegram,Updater} artifact/ 2>/dev/null || true
if [ -f OmniGram-x86_64.AppImage ]; then
  mv OmniGram-x86_64.AppImage artifact/
fi

echo "=== Packaging complete ==="
