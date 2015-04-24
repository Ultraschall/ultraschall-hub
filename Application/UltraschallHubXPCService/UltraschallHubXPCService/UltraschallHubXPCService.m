//
//  UltraschallHubXPCService.m
//  UltraschallHubXPCService
//
//  Created by Daniel Lindenfelser on 23/04/15.
//  Copyright (c) 2015 Daniel Lindenfelser. All rights reserved.
//

#import "UltraschallHubXPCService.h"

@implementation UltraschallHubXPCService

// This implements the example protocol. Replace the body of this class with the implementation of this service's protocol.
- (void)upperCaseString:(NSString *)aString withReply:(void (^)(NSString *))reply {
    NSString *response = [aString uppercaseString];
    reply(response);
}

@end
