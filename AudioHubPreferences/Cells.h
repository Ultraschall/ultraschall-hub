//
//  DeviceTableCellView.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 21/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface DeviceTableCellView : NSTableCellView

@property NSString* deviceUID;
@property NSString* deviceName;

@end

@interface ChannelTableCellView : NSTableCellView

@property NSString* deviceChannels;

@end
