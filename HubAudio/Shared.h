/*
The MIT License (MIT)

Copyright (c) 2015 Daniel Lindenfelser

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef Shared_h
#define Shared_h

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudioTypes.h>

static const CFStringRef kHubBundleIdentifier = CFSTR("de.8plugs.audio.HubAudio");
static const CFStringRef kHubManufacturer = CFSTR("8plugs");

static const CFStringRef kHubBoxName = CFSTR("HubBox");
static const CFStringRef kHubBoxModelName = CFSTR("HubBox");
static const CFStringRef kHubBoxSerialNumber = CFSTR("0815");
static const CFStringRef kHubBoxFirmwareVersion = CFSTR("1.0");
static const CFStringRef kHubBoxUUID = CFSTR("HUB:1");

    
enum {
    kHubCustomPropertySettings = 'hubs'
};
const UInt32 kHubCustomProperties = 1;

static const CFStringRef kHubSettingsKeySettings = CFSTR("HubSettings");
static const CFStringRef kHubSettingsKeyDevices = CFSTR("Devices");
static const CFStringRef kHubSettingsKeyDeviceName = CFSTR("Name");
static const CFStringRef kHubSettingsKeyDeviceUUID = CFSTR("UUID");
static const CFStringRef kHubSettingsKeyDeviceChannels = CFSTR("Channels");

static const UInt32 kHubMaximumDeviceChannels = 32;

#endif /* Shared_h */
