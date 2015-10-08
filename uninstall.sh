#!/bin/sh

#  uninstall.sh
#  AudioHub
#
#  Created by Daniel Lindenfelser on 11/09/15.
#  Copyright © 2015 Daniel Lindenfelser. All rights reserved.

echo "Stop Core Audio Server"
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

if [ -d /Library/Audio/Plug-Ins/HAL/HubAudio.driver ]; then
echo "Remove AudioHub Driver"
sudo rm -rf /Library/Audio/Plug-Ins/HAL/AudioHub.driver > /dev/null
fi

if [ -d /Library/PreferencePanes/AudioHubPreferences.prefPane ]; then
echo "Remove AudioHub Preferences"
sudo rm -rf /Library/PreferencePanes/AudioHubPreferences.prefPane > /dev/null
fi


echo "Start Core Audio Server"
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

