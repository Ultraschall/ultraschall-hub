#!/bin/sh
xcodebuild -project "UltraschallHub.xcodeproj" -configuration Debug -showBuildSettings | sed '1d;s/^ *//;s/"/\\"/g;s/ = \(.*\)/="\1"/;s/ = /=/;s/UID.*//' > xcodebuild-env.tmp
source xcodebuild-env.tmp
rm xcodebuild-env.tmp
sudo cp -rfv $TARGET_BUILD_DIR/$TARGET_NAME.driver /Library/Audio/Plug-Ins/HAL/
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
