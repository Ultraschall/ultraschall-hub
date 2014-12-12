//
//  DNLPlayThroughManager.m
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 14/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import "DNLPlayThroughManager.h"

@implementation DNLPlayThroughAudioDevice

- (instancetype)initWithDeviceId:(AudioDeviceID)deviceID {
    self = [super init];
    if(self != nil)
    {
        self.deviceID = deviceID;
    }
    return self;
}

- (void)updateDeviceInfo {
    [self.outputChannes removeAllObjects];
    [self.inputChannels removeAllObjects];
}

@end

@implementation DNLPlayThroughManager

- (void)scanDevices {
    UInt32 propsize;
    
    AudioObjectPropertyAddress theAddress = { kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster };
    
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &theAddress, 0, NULL, &propsize);
    int nDevices = propsize / sizeof(AudioDeviceID);
    AudioDeviceID *devids = new AudioDeviceID[nDevices];
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress, 0, NULL, &propsize, devids);
    
    for (int i = 0; i < nDevices; ++i) {
        DNLPlayThroughAudioDevice *device = [[DNLPlayThroughAudioDevice alloc] initWithDeviceId:devids[i]];
    }
    delete[] devids;
}

@end
