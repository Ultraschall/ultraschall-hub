#!/bin/sh

sudo cp -rf /Users/danlin/Library/Developer/Xcode/DerivedData/UltraschallHub-djhwxqlajhmvaogxdqcnwlxvxmby/Build/Products/Debug/UltraschallHub.driver /Library/Audio/Plug-Ins/HAL/
sudo killall coreaudiod
