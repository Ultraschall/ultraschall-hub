#!/bin/sh
VERSION="0.9.7.5"
CONFIGURATION="Release"

echo "Build UltraschallHub $VERSION"

if [ -d ./Payload ]; then
	rm -rf ./Payload
fi
mkdir ./Payload

echo "Build Core Audio Server Plug-in"
xcodebuild -project "../Plug-in/UltraschallHubDriver.xcodeproj" -configuration $CONFIGURATION clean > /dev/null
xcodebuild -project "../Plug-in/UltraschallHubDriver.xcodeproj" -configuration $CONFIGURATION build > /dev/null

echo "Create Core Audio Server Plug-in Package"
if [ -d ../Plug-in/Payload ]; then
	rm -rf ../Plug-in/Payload
fi
mkdir ../Plug-in/Payload
cp -rf ../Plug-in/build/$CONFIGURATION/*.driver ../Plug-in/Payload/
pkgbuild --root ../Plug-in/Payload --identifier fm.ultraschall.UltraschallHubDriver --scripts ../Plug-in/Scripts --install-location /Library/Audio/Plug-ins/HAL ./Payload/UltraschallHubDriver.$VERSION.pkg > /dev/null
rm -rf ../Plug-in/Payload

echo "Create Installer"
productbuild --distribution UltraschallHub.plist --package-path Payload --resources Resources ./Package/UltraschallHub.$VERSION.pkg > /dev/null

echo "Create Image"
if [ -f UltraschallHub-$VERSION.dmg.gz ]; then
	rm -f UltraschallHub-$VERSION.dmg.gz
fi

hdiutil create -srcfolder ./Package -volname "UltraschallHub-$VERSION" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 50m pack.temp.dmg > /dev/null
device=$(hdiutil attach -readwrite -noverify -noautoopen "pack.temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $1}') > /dev/null

echo '
   tell application "Finder"
     tell disk "UltraschallHub-'$VERSION'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {300, 200, 100, 100}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 72
           set background picture of theViewOptions to file ".background:Background.png"
           set position of item "UltraschallHub.0.9.7.5.pkg" of container window to {150, 100}
           update without registering applications
           delay 5
           close
     end tell
   end tell
' | osascript > /dev/null

chmod -Rf go-w /Volumes/UltraschallHub-$VERSION
sync
sync
hdiutil detach ${device} > /dev/null
hdiutil convert pack.temp.dmg -format UDZO -imagekey zlib-level=9 -o ./UltraschallHub-$VERSION.dmg > /dev/null
rm -f pack.temp.dmg 

gzip ./UltraschallHub-$VERSION.dmg


echo "CleanUp"
rm -rf ./Payload
rm -f ./Package/UltraschallHub.$VERSION.pkg
rm -f ./UltraschallHub-$VERSION.dmg
