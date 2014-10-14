//
//  MappingView.h
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 07/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DeviceView.h"

@interface MappingView : NSView <DeviceConnector>

- (IBAction)test:(id)sender;
-(void)connectInput:(Conncetor*)input withOutput:(Conncetor*)output;

@end
