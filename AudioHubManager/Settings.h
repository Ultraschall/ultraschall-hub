//
//  AudioHubSettings.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 15/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AudioHubDevice : NSObject
@property NSString* name;
@property NSString* uid;
@property NSInteger channels;
@end

@interface AudioHubSettings : NSObject

- (instancetype)initWithSettings:(NSDictionary *)settings;
- (NSDictionary*)getSettings;

- (void)addDevice:(AudioHubDevice*) device;
- (void)addDevice:(NSString*) name andUID:(NSString*) uid andChannels:(NSInteger) channels;

- (void)removeDevice:(AudioHubDevice*) device;
- (void)removeDeviceWithUID: (NSString*) uid;
- (void)removeDeviceAtIndex: (NSUInteger) index;

- (NSUInteger)numberOfDevices;
- (AudioHubDevice*)deviceAtIndex:(NSUInteger)index;
- (AudioHubDevice*)deviceWithUID:(NSString*)uid;
@end
