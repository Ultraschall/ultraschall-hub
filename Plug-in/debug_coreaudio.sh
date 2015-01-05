#!/bin/sh
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
if [ -d /Library/Audio/Plug-Ins/HAL/UltraschallHub.driver ]; then
	sudo rm -rf /Library/Audio/Plug-Ins/HAL/UltraschallHub.driver
fi
sudo cp -rfv ./build/Debug/UltraschallHub.driver /Library/Audio/Plug-Ins/HAL/
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
