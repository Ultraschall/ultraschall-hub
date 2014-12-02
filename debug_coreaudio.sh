#!/bin/sh

sudo cp -rf /Users/danlin/Library/Developer/Xcode/DerivedData/UltraschallHub-drxkehoevtcjrngbkjwdxytmhtfy/Build/Products/Debug/UltraschallHub.driver /Library/Audio/Plug-Ins/HAL/
sudo killall coreaudiod
