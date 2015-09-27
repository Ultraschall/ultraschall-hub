#!/bin/sh

#  uninstall.sh
#  AudioHub
#
#  Created by Daniel Lindenfelser on 11/09/15.
#  Copyright © 2015 Daniel Lindenfelser. All rights reserved.

echo "Stop CoreAudio Server"
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

if [ -d /Library/Audio/Plug-Ins/HAL/AudioHub.driver ]; then
    echo "Remove AudioHub Driver"
    sudo rm -rf /Library/Audio/Plug-Ins/HAL/AudioHub.driver >/dev/null
fi

echo "Start CoreAudio Server"
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

