//
//  AudioHub.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 14/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#ifndef __AudioHubTypes__
#define __AudioHubTypes__

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudioTypes.h>

static const CFStringRef kAudioHubBundleIdentifier = CFSTR("de.8plugs.audio.AudioHub");
static const CFStringRef kAudioHubManufacturer = CFSTR("8plugs");

static const CFStringRef kAudioHubBoxName = CFSTR("de.8plugs.audio.AudioHubBox");
static const CFStringRef kAudioHubBoxModelName = CFSTR("de.8plugs.audio.AudioHubBox");
static const CFStringRef kAudioHubBoxSerialNumber = CFSTR("0815");
static const CFStringRef kAudioHubBoxFirmwareVersion = CFSTR("1.0");
static const CFStringRef kAudioHubBoxUID = CFSTR("de.8plugs.audio.AudioHub:1");


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


#endif /* __AudioHubTypes__ */
