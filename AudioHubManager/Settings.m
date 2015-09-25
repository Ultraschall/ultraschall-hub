//
//  AudioHubSettings.m
//  AudioHub
//
//  Created by Daniel Lindenfelser on 15/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import "Settings.h"
#include "AudioHubTypes.h"


@implementation AudioHubDevice
@end

@interface AudioHubSettings()
@property NSMutableArray* devices;
@end

@implementation AudioHubSettings

- (instancetype)initWithSettings:(NSDictionary *)settings {
    if (self = [super init]) {
        self.devices = [[NSMutableArray alloc] init];
        [self parseSettings:settings];
    }
    return self;
}

- (id)init {
    if (self = [super init]) {
        self.devices = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void) parseSettings:(NSDictionary*)settings {
    NSArray *devicesArray = [settings objectForKey:(__bridge NSString*)kAudioHubSettingsKeyDevices];
    for (NSDictionary* deviceDictionary in devicesArray) {
        [self addDevice:[deviceDictionary objectForKey:(__bridge NSString*)kAudioHubSettingsKeyDeviceName]
                 andUID:[deviceDictionary objectForKey:(__bridge NSString*)kAudioHubSettingsKeyDeviceUID]
            andChannels:[(NSNumber*)[deviceDictionary objectForKey:(__bridge NSString*)kAudioHubSettingsKeyDeviceChannels] integerValue]];
    }
}

- (NSDictionary*)getSettings {
    NSMutableDictionary *settingsDictionary = [[NSMutableDictionary alloc] init];
    NSMutableArray *devicesArray = [[NSMutableArray alloc] init];
    for (AudioHubDevice* device in self.devices) {
        NSMutableDictionary *deviceDictionary = [[NSMutableDictionary alloc] init];
        [deviceDictionary setObject:device.name forKey:(__bridge NSString*)kAudioHubSettingsKeyDeviceName];
        [deviceDictionary setObject:device.uid forKey:(__bridge NSString*)kAudioHubSettingsKeyDeviceUID];
        [deviceDictionary setObject:[NSNumber numberWithInteger:device.channels] forKey:(__bridge NSString*)kAudioHubSettingsKeyDeviceChannels];
        [devicesArray addObject:deviceDictionary];
    }
    [settingsDictionary setObject:devicesArray forKey:(__bridge NSString*)kAudioHubSettingsKeyDevices];
    return settingsDictionary;
}

- (void)addDevice:(AudioHubDevice *)device {
    [self.devices addObject:device];
}

- (void)addDevice:(NSString*) name andUID:(NSString*) uid andChannels:(NSInteger) channels {
    AudioHubDevice* device = [[AudioHubDevice alloc] init];
    device.name = name;
    device.uid = uid;
    device.channels = channels;
    [self.devices addObject:device];
}

- (void)removeDevice:(AudioHubDevice *)device {
    [self.devices removeObject:device];
}

- (void)removeDeviceWithUID:(NSString *)uid {
    AudioHubDevice* deviceToDelete;
    for (AudioHubDevice* device in self.devices) {
        if ([device.uid compare:uid] == NSOrderedSame) {
            deviceToDelete = device;
            break;
        }
    }
    if (deviceToDelete == nil)
        return;
    
    [self.devices removeObject:deviceToDelete];
}

- (void)removeDeviceAtIndex:(NSUInteger)index {
    [self.devices removeObjectAtIndex:index];
}

- (NSUInteger)numberOfDevices {
    return self.devices.count;
}

- (AudioHubDevice*)deviceAtIndex:(NSUInteger)index {
    return [self.devices objectAtIndex:index];
}

- (AudioHubDevice*)deviceWithUID:(NSString*)uid {
    for (AudioHubDevice* device in self.devices) {
        if ([device.uid compare:uid] == NSOrderedSame) {
            return device;
        }
    }
    return nil;
}

@end
