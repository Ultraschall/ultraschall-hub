//
//  AudioHubViewController.m
//  AudioHub
//
//  Created by Daniel Lindenfelser on 20/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import "AudioHubViewController.h"

@implementation NotFoundView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    [[NSColor controlBackgroundColor] setFill];
    NSRectFill(dirtyRect);
}

@end

@interface AudioHubViewController ()

@end

@implementation AudioHubViewController

- (void)loadSettingsFromPlugin {
    self.manager = [[AudioHubManager alloc] init];
    self.settings = [[AudioHubSettings alloc] init];
    NSError *error;
    if (![self.manager isPluginReady:&error]) {
        NSLog(@"Audio Hub Error: %@", error.description);
        self.notFoundView.frame = self.view.frame;
        [self.view addSubview:self.notFoundView positioned:NSWindowAbove relativeTo:nil];
    }
    self.settings = [self.manager getCurrentSettings:&error];
    if (self.settings == nil) {
        NSLog(@"Audio Hub Error: %@", error.description);
        self.settings = [[AudioHubSettings alloc] init];
    }
    [self.tableView reloadData];
    self.isChanged = NO;
}

- (void)updateStatus {
    if ([self.manager isBoxActive]) {
        self.audioHubStatus.stringValue = @"Audio Hub: On";
        self.audioHubStatusButton.title = @"Turn Audio Hub Off";
    } else {
        self.audioHubStatus.stringValue = @"Audio Hub: Off";
        self.audioHubStatusButton.title = @"Turn Audio Hub On";
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self loadSettingsFromPlugin];
    [self updateStatus];
}

- (IBAction)doubleAction:(id)sender {
    if (self.tableView.numberOfSelectedRows> 0) {
        [self changeDevice:sender];
    }
}

- (IBAction)addDevice:(id)sender {
    self.deviceNameTextField.stringValue = @"";
    self.deviceUIDTextField.stringValue = @"";
    self.deviceUIDTextField.placeholderString = [NSString stringWithFormat:@"AudioHubDevice:%lu", (unsigned long)self.settings.numberOfDevices];
    [self.deviceChannelsPopUpButton selectItemWithTag:2];
    [self.view.window beginSheet:self.deviceSheet completionHandler:^(NSModalResponse returnCode) {
        if (returnCode == NSModalResponseCancel)
            return;
        [self.settings addDevice:self.deviceNameTextField.stringValue
                          andUID:[self.deviceUIDTextField.stringValue isEqualTo:@""] ? self.deviceUIDTextField.placeholderString : self.deviceUIDTextField.stringValue
                     andChannels:self.deviceChannelsPopUpButton.selectedItem.tag];
        NSInteger newRowIndex = self.settings.numberOfDevices - 1;
        [self.tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:newRowIndex] withAnimation:NSTableViewAnimationEffectGap];
        [self.tableView selectRowIndexes:[NSIndexSet indexSetWithIndex:newRowIndex] byExtendingSelection:false];
        [self.tableView scrollRowToVisible:newRowIndex];
        [self.tableView becomeFirstResponder];
        self.isChanged = YES;
    }];
}

- (IBAction)removeDevice:(id)sender {
    if (self.tableView.selectedRow != -1) {
        NSInteger rowIndex = self.settings.numberOfDevices - 1;
        AudioHubDevice* device = [self.settings deviceAtIndex:rowIndex];
        if (device != nil) {
            [self.settings removeDeviceAtIndex:rowIndex];
            [self.tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:self.tableView.selectedRow] withAnimation:NSTableViewAnimationEffectFade];
            self.isChanged = YES;
        }
    }
}

- (IBAction)changeDevice:(id)sender {
    if (self.tableView.selectedRow != -1) {
        NSInteger rowIndex = self.tableView.selectedRow;
        AudioHubDevice* device = [self.settings deviceAtIndex:rowIndex];
        if (device != nil) {
            self.deviceNameTextField.stringValue = device.name;
            self.deviceUIDTextField.stringValue = device.uid;
            [self.deviceChannelsPopUpButton selectItemWithTag:device.channels];
            
            [self.view.window beginSheet:self.deviceSheet completionHandler:^(NSModalResponse returnCode) {
                if (returnCode == NSModalResponseCancel)
                    return;
                
                device.name = self.deviceNameTextField.stringValue;
                device.uid = [self.deviceUIDTextField.stringValue isEqualTo:@""] ? self.deviceUIDTextField.placeholderString : self.deviceUIDTextField.stringValue;
                device.channels = self.deviceChannelsPopUpButton.selectedTag;
                [self.tableView reloadData];
                self.isChanged = YES;
            }];
        }
    }
}

- (IBAction)importConfiguration:(id)sender {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = @[@"ahc"];
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
        if (returnCode == NSModalResponseCancel)
            return;
        
        NSURL *file = [openPanel URL];
        NSDictionary *settings = [NSDictionary dictionaryWithContentsOfURL:file];
        if (settings != nil) {
            self.settings = [[AudioHubSettings alloc] initWithSettings:settings];
            [self.tableView reloadData];
            self.isChanged = YES;
        }
    }];
}

- (IBAction)exportConfiguration:(id)sender {
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    savePanel.allowedFileTypes = @[@"ahc"];
    [savePanel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
        if (returnCode == NSModalResponseCancel)
            return;
        
        NSURL *file = [savePanel URL];
        
        NSDictionary* settings = [self.settings getSettings];
        [settings writeToURL:file atomically:YES];
    }];
}

- (IBAction)toggleAudioHubState:(id)sender {
    [self.manager setBoxActive:![self.manager isBoxActive]];
    [self updateStatus];
}

- (IBAction)apply:(id)sender {
    NSDictionary* settings = [self.settings getSettings];
    if (settings != nil) {
        [self.manager uploadSettings:self.settings];
    }
    self.isChanged = NO;
}

- (IBAction)revert:(id)sender {
    [self loadSettingsFromPlugin];
}

#pragma mark Device Sheet

- (IBAction)okButtonClicked:(id)sender {
    if ([self.deviceNameTextField.stringValue isEqualTo:@""]) {
        [self.deviceNameTextField becomeFirstResponder];
        return;
    }
    [self.view.window endSheet:self.deviceSheet returnCode:NSModalResponseOK];
}

- (IBAction)cancelButtonClicked:(id)sender {
    [self.view.window endSheet:self.deviceSheet returnCode:NSModalResponseCancel];
}

#pragma mark Table View

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return self.settings.numberOfDevices;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    NSString *columnIdentifer = [tableColumn identifier];
    NSView* cellView = [tableView makeViewWithIdentifier:columnIdentifer owner:self];
    if (cellView != nil) {
        AudioHubDevice *device = [self.settings deviceAtIndex:row];
        if (device != nil) {
            if ([tableColumn.identifier isEqualToString:@"device"]) {
                DeviceTableCellView *deviceTableCellView = (DeviceTableCellView*)cellView;
                deviceTableCellView.deviceName = device.name;
                deviceTableCellView.deviceUID = device.uid;
            } else if ([tableColumn.identifier isEqualToString:@"channel"]) {
                ChannelTableCellView *channelTableCellView = (ChannelTableCellView*)cellView;
                channelTableCellView.deviceChannels = [NSString stringWithFormat:@"%lu", (unsigned long)device.channels];
            }
        }
    }
    return cellView;
}

@end
