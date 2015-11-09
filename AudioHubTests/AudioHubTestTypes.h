//
//  AudioHubTestTypes.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 09/11/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#ifndef AudioHubTestTypes_h
#define AudioHubTestTypes_h

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudioTypes.h>

static const CFStringRef kAudioHubBundleIdentifier = CFSTR("de.8plugs.audio.AudioHubTests");
static const CFStringRef kAudioHubManufacturer = CFSTR("8plugs");

static const CFStringRef kAudioHubBoxName = CFSTR("de.8plugs.audio.AudioHubBox");
static const CFStringRef kAudioHubBoxModelName = CFSTR("de.8plugs.audio.AudioHubBox");
static const CFStringRef kAudioHubBoxSerialNumber = CFSTR("0815");
static const CFStringRef kAudioHubBoxFirmwareVersion = CFSTR("1.0");
static const CFStringRef kAudioHubBoxUID = CFSTR("de.8plugs.audio.AudioHub:1");

static const CFStringRef kAudioHubDeviceModelUID = CFSTR("de.8plugs.audio.AudioHubDevice");


enum {
    kAudioHubCustomPropertySettings = 'ephs',
    kAudioHubCustomPropertyActive = 'epha'
};
const UInt32 kAudioHubCustomProperties = 2;

static const CFStringRef kAudioHubSettingsKey = CFSTR("AudioHubSettings");
static const CFStringRef kAudioHubSettingsKeyDevices = CFSTR("AudioHubDevices");
static const CFStringRef kAudioHubSettingsKeyDeviceName = CFSTR("Name");
static const CFStringRef kAudioHubSettingsKeyDeviceUID = CFSTR("UID");
static const CFStringRef kAudioHubSettingsKeyDeviceChannels = CFSTR("Channels");

static const UInt32 kAudioHubMaximumDeviceChannels = 32;




#endif /* AudioHubTestTypes_h */
