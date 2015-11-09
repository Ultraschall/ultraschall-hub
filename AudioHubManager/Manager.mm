//
//  Manager.m
//  AudioHub
//
//  Created by Daniel Lindenfelser on 15/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import "Manager.h"
#import <CoreAudio/CoreAudio.h>
#include "AudioHubTypes.h"
#include "CAHALAudioSystemObject.h"
#include "CAPropertyAddress.h"

@implementation AudioHubManager

- (NSError*)pluginNotFoundError {
    return [NSError errorWithDomain:(__bridge NSString*)kAudioHubBundleIdentifier code:200 userInfo:@{@"Unable to find the plugin": @""}];
}

- (NSError*)boxNotFoundError {
    return [NSError errorWithDomain:(__bridge NSString*)kAudioHubBundleIdentifier code:201 userInfo:@{@"Unable to find the plugins box": @""}];
}

- (NSError*)cantReadSettingsError {
    return [NSError errorWithDomain:(__bridge NSString*)kAudioHubBundleIdentifier code:202 userInfo:@{@"Unable to read settings": @""}];
}

- (NSError*)invalideSettingsError {
    return [NSError errorWithDomain:(__bridge NSString*)kAudioHubBundleIdentifier code:202 userInfo:@{@"Unable to read settings": @""}];
}

#pragma mark Init
- (id)init {
    if (self = [super init]) {
    }
    return self;
}

#pragma mark Manager
- (AudioObjectID) getBox: (NSError**)error {
    CAHALAudioSystemObject audioSystemObject;
    AudioObjectID pluginAudioObjectID = audioSystemObject.GetAudioPlugInForBundleID(kAudioHubBundleIdentifier);
    if (pluginAudioObjectID == kAudioObjectUnknown) {
        *error = [self pluginNotFoundError];
        return kAudioObjectUnknown;
    }
    
    CAHALAudioObject pluginAudioObject(pluginAudioObjectID);
    CAPropertyAddress address(kAudioPlugInPropertyBoxList);
    if (!pluginAudioObject.HasProperty(address)) {
        *error = [self pluginNotFoundError];
        return kAudioObjectUnknown;
    }
    UInt32 size = pluginAudioObject.GetPropertyData_ArraySize<AudioObjectID>(address);
    if (size <= 0) {
        *error = [self pluginNotFoundError];
        return kAudioObjectUnknown;
    }
    AudioObjectID data[size];
    pluginAudioObject.GetPropertyData_Array<AudioObjectID>(address, size, data);
    AudioObjectID boxAudioObjectID = data[0];
    if (boxAudioObjectID == kAudioObjectUnknown) {
        *error = [self boxNotFoundError];
        return kAudioObjectUnknown;
    }
    return boxAudioObjectID;
}

- (BOOL) isPluginReady: (NSError**)error {
    if ([self getBox:error] == kAudioObjectUnknown)
        return false;
    return true;
}

- (BOOL) isBoxActive {
    NSError* error;
    AudioObjectID boxID = [self getBox:&error];
    if (boxID == kAudioObjectUnknown)
        return false;
    
    CAHALAudioObject boxAudioObject(boxID);
    CAPropertyAddress address(kAudioBoxPropertyAcquired);
    UInt32 acquired = boxAudioObject.GetPropertyData_UInt32(address);
    
    if (!acquired) {
        return false;
    }
    return true;
}

- (void) setBoxActive: (BOOL) active {
    NSError* error;
    AudioObjectID boxID = [self getBox:&error];
    if (boxID == kAudioObjectUnknown)
        return;
    
    CAHALAudioObject boxAudioObject(boxID);
    NSString *state;
    CAPropertyAddress address(kAudioHubCustomPropertyActive);
    if (active)
        state = @"YES";
    else
        state = @"NO";
    boxAudioObject.SetPropertyData_CFString(address, (__bridge CFStringRef)state);
}

- (AudioHubSettings*) getCurrentSettings: (NSError**)error {
    AudioObjectID boxAudioObjectID = [self getBox:error];
    if (boxAudioObjectID == kAudioObjectUnknown)
        return nil;
    
    CAHALAudioObject boxAudioObject(boxAudioObjectID);
    CAPropertyAddress address(kAudioHubCustomPropertySettings);
    CFPropertyListRef result = boxAudioObject.GetPropertyData_CFType(address);
    if (result == NULL) {
        *error = [self cantReadSettingsError];
        return nil;
    }
    
    if (CFGetTypeID(result) != CFDictionaryGetTypeID()) {
        *error = [self invalideSettingsError];
        return nil;
    }
    
    NSDictionary *settignsDictionary = (__bridge NSDictionary*)result;
    AudioHubSettings *settings = [[AudioHubSettings alloc] initWithSettings:settignsDictionary];
    
    if (settings == nil) {
        *error = [self invalideSettingsError];
        return nil;
    }
    
    return settings;
}

- (void) uploadSettings: (AudioHubSettings*) settings {
    NSError* error;
    AudioObjectID boxAudioObjectID = [self getBox:&error];
    if (boxAudioObjectID == kAudioObjectUnknown)
        return;
    
    CAHALAudioObject boxAudioObject(boxAudioObjectID);
    CAPropertyAddress address(kAudioHubCustomPropertySettings);
    CFDictionaryRef data = (__bridge CFDictionaryRef)[settings getSettings];
    CFRetain(data);
    boxAudioObject.SetPropertyData_CFType(address, data);
    CFRelease(data);
}



@end
