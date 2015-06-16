//
//  PlugInTests.m
//  UltraschallHubDriver
//
//  Created by Daniel Lindenfelser on 15/06/15.
//  Copyright © 2015 ultraschall.fm. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <CoreAudio/AudioServerPlugIn.h>
#include "CAAutoDisposer.h"
#include "CAHALAudioSystemObject.h"
#include "CAPropertyAddress.h"

#define uuid CFSTR("UltraschallHub:94AF43F5")

@interface PlugInTests : XCTestCase
@property CAHALAudioSystemObject systemObject;
@property AudioObjectID pluginObjectId;
@property CAHALAudioObject* pluginObject;
@end

//  kAudioPlugInPropertyBundleID                = 'piid' OK
//  kAudioPlugInPropertyDeviceList              = 'dev#' OK
//  kAudioPlugInPropertyTranslateUIDToDevice    = 'uidd' OK
//  kAudioPlugInPropertyBoxList                 = 'box#' NOT IMPLEMENTED
//  kAudioPlugInPropertyTranslateUIDToBox       = 'uidb' NOT IMPLEMENTED
//
//  kAudioPlugInPropertyResourceBundle          = 'rsrc' OK

@implementation PlugInTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    self.pluginObjectId = self.systemObject.GetAudioPlugInForBundleID(CFSTR("fm.ultraschall.audio.UltraschallHub"));
    self.pluginObject = new CAHALAudioObject(self.pluginObjectId);
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
    
}

#pragma mark Audio Object Properties

- (void)testAudioObjectPropertyName {
    CAPropertyAddress address(kAudioObjectPropertyName);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.pluginObject->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

- (void)testAudioObjectPropertyOwnedObjects {
    CAPropertyAddress address(kAudioObjectPropertyOwnedObjects);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 ioNumberItems = self.pluginObject->GetPropertyData_ArraySize<AudioObjectID>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<AudioObjectID> outArray(ioNumberItems);
    self.pluginObject->GetPropertyData_Array<AudioObjectID>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
}

#pragma mark Audio PlugIn Properties

- (void)testAudioPlugInPropertyBundleID {
    CAPropertyAddress address(kAudioPlugInPropertyBundleID);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.pluginObject->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

- (void)testAudioPlugInPropertyDeviceList {
    CAPropertyAddress address(kAudioPlugInPropertyDeviceList);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 ioNumberItems = self.pluginObject->GetPropertyData_ArraySize<AudioObjectID>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);

    CAAutoArrayDelete<AudioObjectID> outArray(ioNumberItems);
    self.pluginObject->GetPropertyData_Array<AudioObjectID>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
}

- (void)testAudioPlugInPropertyTranslateUIDToDevice  {
    CAPropertyAddress address(kAudioPlugInPropertyTranslateUIDToDevice);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    AudioObjectID data = 0;
    UInt32 inQualifierDataSize = sizeof(CFStringRef);
    CFStringRef inQualifierData = uuid;
    self.pluginObject->GetPropertyData(address, inQualifierDataSize, &inQualifierData, dataSize, &data);
    XCTAssertGreaterThan(data, 0);
}

- (void)testAudioPlugInPropertyBoxList {
    CAPropertyAddress address(kAudioPlugInPropertyBoxList);
    XCTAssertFalse(self.pluginObject->HasProperty(address));
}

- (void)testAudioPlugInPropertyTranslateUIDToBox {
    CAPropertyAddress address(kAudioPlugInPropertyTranslateUIDToBox);
    XCTAssertFalse(self.pluginObject->HasProperty(address));
}

- (void)testAudioPlugInPropertyResourceBundle {
    CAPropertyAddress address(kAudioPlugInPropertyResourceBundle);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.pluginObject->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

@end
