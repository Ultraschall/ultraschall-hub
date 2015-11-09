//
//  UltraschallHubTypes.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 09/11/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#ifndef UltraschallHubTypes_h
#define UltraschallHubTypes_h

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudioTypes.h>

static const CFStringRef kAudioHubBundleIdentifier = CFSTR("fm.ultraschall.audio.UltraschallHub");
static const CFStringRef kAudioHubManufacturer = CFSTR("ultraschall");

static const CFStringRef kAudioHubBoxName = CFSTR("fm.ultraschall.audio.UltraschallHubBox");
static const CFStringRef kAudioHubBoxModelName = CFSTR("fm.ultraschall.audio.UltraschallHubBox");
static const CFStringRef kAudioHubBoxSerialNumber = CFSTR("0815");
static const CFStringRef kAudioHubBoxFirmwareVersion = CFSTR("1.0");
static const CFStringRef kAudioHubBoxUID = CFSTR("fm.ultraschall.audio.UltraschallHub:1");

static const CFStringRef kAudioHubDeviceModelUID = CFSTR("fm.ultraschall.audio.UltraschallHubDevice");

const UInt32 kAudioHubCustomProperties = 0;

static const CFStringRef kAudioHubSettingsKey = CFSTR("UltraschallHubSettings");
static const CFStringRef kAudioHubSettingsKeyDevices = CFSTR("UltraschallHubDevices");
static const CFStringRef kAudioHubSettingsKeyDeviceName = CFSTR("Name");
static const CFStringRef kAudioHubSettingsKeyDeviceUID = CFSTR("UID");
static const CFStringRef kAudioHubSettingsKeyDeviceChannels = CFSTR("Channels");

static const UInt32 kAudioHubMaximumDeviceChannels = 32;

#endif /* UltraschallHubTypes_h */
