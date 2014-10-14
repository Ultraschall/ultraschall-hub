//
//  DeviceView.h
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 07/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <CoreAudio/CoreAudio.h>

typedef enum {
    kConncetorNone,
    kConncetorOutput,
    kConncetorInput
} ConncetorType;

@protocol DeviceConnector;

@class DeviceView;

@interface Conncetor : NSObject

@property ConncetorType type;

@property NSString *title;
@property NSObject *tag;

@property Conncetor* connectedTo;

@property DeviceView* superview;

@property AudioDeviceID deviceID;
@property UInt32 channel;

@end

@interface DeviceView : NSView

@property NSMutableArray *outputs;
@property NSMutableArray *inputs;
@property BOOL drag;

@property BOOL dragConnection;
@property Conncetor* dragConnectionObject;
@property NSPoint dragConnectionStartPoint;
@property BOOL dragValideConnection;

@property (nonatomic, weak) id<DeviceConnector> delegate;

- (void)setTitle:(NSString *)titleText;
- (Conncetor *)addOutputWithTitle:(NSString*)title;
- (Conncetor *)addOutput:(Conncetor *)connector;
- (Conncetor *)addInputWithTitle:(NSString*)title;
- (Conncetor *)addInput:(Conncetor *)connector;

- (void)drawLinks:(NSRect)frame;

@end

@protocol DeviceConnector <NSObject>
@required
-(void)connectInput:(Conncetor*)input withOutput:(Conncetor*)output;
@end


