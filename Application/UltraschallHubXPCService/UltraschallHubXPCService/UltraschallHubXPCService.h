//
//  UltraschallHubXPCService.h
//  UltraschallHubXPCService
//
//  Created by Daniel Lindenfelser on 23/04/15.
//  Copyright (c) 2015 Daniel Lindenfelser. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "UltraschallHubXPCServiceProtocol.h"

// This object implements the protocol which we have defined. It provides the actual behavior for the service. It is 'exported' by the service to make it available to the process hosting the service over an NSXPCConnection.
@interface UltraschallHubXPCService : NSObject <UltraschallHubXPCServiceProtocol>
@end
