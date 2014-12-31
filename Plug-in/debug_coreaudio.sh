#!/bin/sh
xcodebuild -project "UltraschallHub.xcodeproj" -configuration Debug clean
xcodebuild -project "UltraschallHub.xcodeproj" -configuration Debug build

sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
if [ -d /Library/Audio/Plug-Ins/HAL/UltraschallHub.driver ]; then
	rm -rf /Library/Audio/Plug-Ins/HAL/UltraschallHub.driver
fi
sudo cp -rfv ./build/Debug/$TARGET_NAME.driver /Library/Audio/Plug-Ins/HAL/
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
