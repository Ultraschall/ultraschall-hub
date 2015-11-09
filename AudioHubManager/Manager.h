//
//  Manager.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 15/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioHubManager/Settings.h>

@protocol AudioHubManagerHostDelegate <NSObject>
@optional
- (void)pluginReadyStateChanged:(BOOL)ready;
@end

@interface AudioHubManager : NSObject

@property (weak) id<AudioHubManagerHostDelegate> delegate;

- (BOOL) isPluginReady: (NSError**)error;
- (BOOL) isBoxActive;
- (void) setBoxActive: (BOOL) active;
- (AudioHubSettings*) getCurrentSettings: (NSError**)error;
- (void) uploadSettings: (AudioHubSettings*) settings;
@end

