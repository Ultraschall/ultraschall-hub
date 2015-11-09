#!/bin/sh

#  install.sh
#  AudioHub
#
#  Created by Daniel Lindenfelser on 11/09/15.
#  Copyright © 2015 Daniel Lindenfelser. All rights reserved.

echo "Stop CoreAudio Server"
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

echo "Start CoreAudio Server"
sudo launchctl load /System/Library/LaunchDaemons/com.apple.audio.coreaudiod.plist

