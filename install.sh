#!/bin/sh

#  install.sh
#  AudioHub
#
#  Created by Daniel Lindenfelser on 11/09/15.
#  Copyright © 2015 Daniel Lindenfelser. All rights reserved.

echo "Stop Core Audio Server"
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

echo "Info"
spctl -a -v /tmp/AudioHub.dst/Library/Audio/Plug-Ins/HAL/AudioHub.driver
spctl -a -v /tmp/AudioHub.dst/Library/PreferencePanes/AudioHubPreferences.prefPane

if [ -d /Library/Audio/Plug-Ins/HAL/AudioHub.driver ]; then
    echo "Remove AudioHub Driver"
    sudo rm -rf /Library/Audio/Plug-Ins/HAL/AudioHub.driver > /dev/null
fi

echo "Update AudioHub Driver"
sudo cp -rfv /tmp/AudioHub.dst/Library/Audio/Plug-Ins/HAL/AudioHub.driver /Library/Audio/Plug-Ins/HAL/ > /dev/null


if [ -d /Library/PreferencePanes/AudioHubPreferences.prefPane ]; then
    echo "Remove AudioHub Preferences"
    sudo rm -rf /Library/PreferencePanes/AudioHubPreferences.prefPane > /dev/null
fi

echo "Update AudioHub Preferences"
sudo cp -rfv /tmp/AudioHub.dst/Library/PreferencePanes/AudioHubPreferences.prefPane /Library/PreferencePanes/ > /dev/null

echo "Start Core Audio Server"
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

