//
//  DeviceView.h
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 07/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    kConncetorNone,
    kConncetorOutput,
    kConncetorInput
} ConncetorType;

@class DeviceView;

@interface Conncetor : NSObject

@property ConncetorType type;

@property NSString *title;
@property NSObject *tag;

@property Conncetor* connectedTo;

@property DeviceView* superview;

@end

@interface DeviceView : NSView

@property NSMutableArray *outputs;
@property NSMutableArray *inputs;
@property BOOL drag;

@property BOOL dragConnection;
@property Conncetor* dragConnectionObject;
@property NSPoint dragConnectionStartPoint;
@property BOOL dragValideConnection;

@property id delegate;

- (void)setTitle:(NSString *)titleText;
- (void)addOutputWithTitle:(NSString*)title;
- (void)addOutput:(Conncetor *)connector;
- (void)addInputWithTitle:(NSString*)title;
- (void)addInput:(Conncetor *)connector;

- (void)drawLinks:(NSRect)frame;

@end


