//
//  MappingView.m
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 07/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import "MappingView.h"

@implementation MappingView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    NSBezierPath* rectanglePath = [NSBezierPath bezierPathWithRect: dirtyRect];
    [NSColor.lightGrayColor setFill];
    [rectanglePath fill];
    // Drawing code here.
    for (NSView *view in self.subviews) {
        if ([view class] == [DeviceView class]) {
            DeviceView* deviceView = (DeviceView*)view;
            [deviceView drawLinks:dirtyRect];
        }
    }
}

- (IBAction)test:(id)sender {
    AudioDeviceList *deviceList = new AudioDeviceList(true);
    AudioDeviceList::DeviceList &thelist = deviceList->GetList();
    
    int index = 0;
    for (AudioDeviceList::DeviceList::iterator i = thelist.begin(); i != thelist.end(); ++i, ++index) {
        DeviceView *newDevice = [[DeviceView alloc] initWithFrame:NSMakeRect(10, 10, 100, 100)];
        [newDevice setTitle:[NSString stringWithCString: (*i).mName encoding:NSASCIIStringEncoding]];
        [self addSubview:newDevice];
        [newDevice addOutputWithTitle:@"Channel 1"];
        [newDevice addOutputWithTitle:@"Channel 2"];
        
        [newDevice addInputWithTitle:@"Channel 1"];
        [newDevice addInputWithTitle:@"Channel 2"];
    }
}

@end
