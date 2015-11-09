//
//  AudioHubViewController.h
//  AudioHub
//
//  Created by Daniel Lindenfelser on 20/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Manager.h"
#import "Settings.h"
#import "Cells.h"

@interface NotFoundView : NSView
@end

@interface AudioHubViewController : NSViewController <NSTableViewDataSource, NSTableViewDelegate>

#pragma mark Audio Hub Manager

@property AudioHubManager* manager;
@property AudioHubSettings* settings;
@property BOOL isChanged;

@property (weak) IBOutlet NSView *notFoundView;
@property (weak) IBOutlet NSTableView *tableView;

@property (weak) IBOutlet NSTextField *audioHubStatus;
@property (weak) IBOutlet NSButton *audioHubStatusButton;
#pragma mark Device Sheet
@property (weak) IBOutlet NSWindow *deviceSheet;
@property (weak) IBOutlet NSTextField *deviceUIDTextField;
@property (weak) IBOutlet NSTextField *deviceNameTextField;
@property (weak) IBOutlet NSPopUpButton *deviceChannelsPopUpButton;

@property BOOL canUpload;

@end
