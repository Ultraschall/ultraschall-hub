//
//  MappingView.m
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 07/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import "MappingView.h"

#import "CAPlayThrough.h"
#import "AudioDevice.h"
#import "AudioDeviceList.h"

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

-(void)connectInput:(Conncetor*)input withOutput:(Conncetor*)output {
    CAPlayThroughHost *pt = new CAPlayThroughHost(input.deviceID, output.deviceID, input.channel, output.channel);
    pt->Start();
}

- (IBAction)test:(id)sender {
    NSMutableDictionary *devices = [[NSMutableDictionary alloc] init];
    
    // Inputs
    bool input = false;
    AudioDeviceList *deviceList = new AudioDeviceList(input);
    AudioDeviceList::DeviceList &thelist = deviceList->GetList();
    
    int index = 0;
    for (AudioDeviceList::DeviceList::iterator i = thelist.begin(); i != thelist.end(); ++i, ++index) {
        NSString *key = [NSString stringWithCString: (*i).mName encoding:NSASCIIStringEncoding];
        if (key != nil) {
            DeviceView *device;
            if ([devices objectForKey:key] != nil) {
                device = (DeviceView*) [devices objectForKey:key];
                AudioDevice *audioDevice = new AudioDevice((*i).mID, input);
                for (int j = 0; j < audioDevice->CountChannels(); j++) {
                    char channelNameBuffer[1024];
                    audioDevice->GetChannelName(channelNameBuffer, sizeof(channelNameBuffer), j);
                    NSString *channelName = [NSString stringWithCString: channelNameBuffer encoding:NSASCIIStringEncoding];
                    Conncetor* connector = [device addInputWithTitle:channelName];
                    [connector setDeviceID:(*i).mID];
                    [connector setChannel:j];
                }
            } else {
                device = [[DeviceView alloc] initWithFrame:NSMakeRect(10, 10, 100, 100)];
                [device setTitle:key];
                AudioDevice *audioDevice = new AudioDevice((*i).mID, input);
                for (int j = 0; j < audioDevice->CountChannels(); j++) {
                    char channelNameBuffer[1024];
                    audioDevice->GetChannelName(channelNameBuffer, sizeof(channelNameBuffer), j);
                    NSString *channelName = [NSString stringWithCString: channelNameBuffer encoding:NSASCIIStringEncoding];
                    Conncetor* connector = [device addInputWithTitle:channelName];
                    [connector setDeviceID:(*i).mID];
                    [connector setChannel:j];
                }
                [self addSubview:device];
                [device setDelegate:self];
                [devices setObject:device forKey:key];
            }
        }
    }
    
    // Outputs
    input = true;
    deviceList = new AudioDeviceList(input);
    thelist = deviceList->GetList();
    
    index = 0;
    for (AudioDeviceList::DeviceList::iterator i = thelist.begin(); i != thelist.end(); ++i, ++index) {
        NSString *key = [NSString stringWithCString: (*i).mName encoding:NSASCIIStringEncoding];
        if (key != nil) {
            DeviceView *device;
            if ([devices objectForKey:key] != nil) {
                device = (DeviceView*) [devices objectForKey:key];
                AudioDevice *audioDevice = new AudioDevice((*i).mID, input);
                for (int j = 0; j < audioDevice->CountChannels(); j++) {
                    char channelNameBuffer[1024];
                    audioDevice->GetChannelName(channelNameBuffer, sizeof(channelNameBuffer), j);
                    NSString *channelName = [NSString stringWithCString: channelNameBuffer encoding:NSASCIIStringEncoding];
                    Conncetor* connector = [device addOutputWithTitle:channelName];
                    [connector setDeviceID:(*i).mID];
                    [connector setChannel:j];
                }
            } else {
                device = [[DeviceView alloc] initWithFrame:NSMakeRect(10, 10, 100, 100)];
                [device setTitle:key];
                AudioDevice *audioDevice = new AudioDevice((*i).mID, input);
                for (int j = 0; j < audioDevice->CountChannels(); j++) {
                    char channelNameBuffer[1024];
                    audioDevice->GetChannelName(channelNameBuffer, sizeof(channelNameBuffer), j);
                    NSString *channelName = [NSString stringWithCString: channelNameBuffer encoding:NSASCIIStringEncoding];
                    Conncetor* connector = [device addOutputWithTitle:channelName];
                    [connector setDeviceID:(*i).mID];
                    [connector setChannel:j];
                }
                [self addSubview:device];
                [device setDelegate:self];
                [devices setObject:device forKey:key];
            }
        }
    }
}

@end
