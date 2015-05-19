#!/bin/sh
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
if [ -d /Library/Audio/Plug-Ins/HAL/UltraschallHub.driver ]; then
	sudo rm -rf /Library/Audio/Plug-Ins/HAL/UltraschallHub.driver
fi
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist
