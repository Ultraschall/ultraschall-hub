//
//  DNLPlayThroughManager.h
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 14/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudio.h>

@interface DNLPlayThroughAudioDevice : NSObject

- (instancetype)initWithDeviceId:(AudioDeviceID)deviceID;

- (void)updateDeviceInfo;

@property AudioDeviceID deviceID;
@property NSString *deviceName;

@property NSMutableArray *inputChannels;
@property NSMutableArray *outputChannes;

@end

@interface DNLPlayThroughInstance : NSObject

@end

@interface DNLPlayThroughManager : NSObject

- (void)scanDevices;

@end
