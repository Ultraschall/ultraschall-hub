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

#pragma mark kAudioObjectPropertyName
// A CFString that contains the human readable name of the object. The caller
// is responsible for releasing the returned CFObject.
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

#pragma mark kAudioObjectPropertyOwnedObjects
// An array of AudioObjectIDs that represent all the AudioObjects owned by the
// given object. The qualifier is an array of AudioClassIDs. If it is
// non-empty, the returned array of AudioObjectIDs will only refer to objects
// whose class is in the qualifier array or whose is a subclass of one in the
// qualifier array.
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

#pragma mark kAudioObjectPropertyManufacturer
// A CFString that contains the human readable name of the manufacturer of the
// hardware the AudioObject is a part of. The caller is responsible for
// releasing the returned CFObject.
- (void)testAudioObjectPropertyManufacturer {
    CAPropertyAddress address(kAudioObjectPropertyManufacturer);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.pluginObject->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
    CFRelease(data);
}

#pragma mark Audio PlugIn Properties

#pragma mark kAudioPlugInPropertyBundleID
// A CFString that contains the bundle identifier for the AudioPlugIn. The
// caller is responsible for releasing the returned CFObject.
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

#pragma mark kAudioPlugInPropertyDeviceList
// An array of AudioObjectIDs that represent all the AudioDevices currently
// provided by the plug-in.
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
    outArray.free();
}

#pragma mark kAudioPlugInPropertyTranslateUIDToDevice
// This property fetches the AudioObjectID that corresponds to the AudioDevice
// that has the given UID. The UID is passed in via the qualifier as a CFString
// while the AudioObjectID for the AudioDevice is returned to the caller as the
// property's data. Note that an error is not returned if the UID doesn't refer
// to any AudioDevices. Rather, this property will return kAudioObjectUnknown
// as the value of the property.
- (void)testAudioPlugInPropertyTranslateUIDToDevice  {
    CAPropertyAddress address(kAudioPlugInPropertyTranslateUIDToDevice);
    XCTAssertTrue(self.pluginObject->HasProperty(address));
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    AudioObjectID data = 0;
    UInt32 inQualifierDataSize = sizeof(CFStringRef);
    CFStringRef inQualifierData = uuid;
    self.pluginObject->GetPropertyData(address, inQualifierDataSize, &inQualifierData, dataSize, &data);
    XCTAssertNotEqual(data, kAudioObjectUnknown);
    
    data = 0;
    inQualifierDataSize = sizeof(CFStringRef);
    inQualifierData = CFSTR("FAIL");
    self.pluginObject->GetPropertyData(address, inQualifierDataSize, &inQualifierData, dataSize, &data);
    XCTAssertEqual(data, kAudioObjectUnknown);
}

#pragma mark kAudioPlugInPropertyBoxList
// An array of AudioObjectIDs that represent all the AudioBox objects currently
// provided by the plug-in.
- (void)testAudioPlugInPropertyBoxList {
    CAPropertyAddress address(kAudioPlugInPropertyBoxList);
    XCTAssertFalse(self.pluginObject->HasProperty(address));
}

#pragma mark kAudioPlugInPropertyTranslateUIDToBox
// This property fetches the AudioObjectID that corresponds to the AudioBox
// that has the given UID. The UID is passed in via the qualifier as a CFString
// while the AudioObjectID for the AudioBox is returned to the caller as the
// property's data. Note that an error is not returned if the UID doesn't refer
// to any AudioBoxes. Rather, this property will return kAudioObjectUnknown
// as the value of the property.
- (void)testAudioPlugInPropertyTranslateUIDToBox {
    CAPropertyAddress address(kAudioPlugInPropertyTranslateUIDToBox);
    XCTAssertFalse(self.pluginObject->HasProperty(address));
}

#pragma mark kAudioPlugInPropertyResourceBundle
// A CFString that contains a path to a resource bundle to use for localizing
// strings and fetching resources like icons from the client process. The path
// is relative to the path of the plug-in's bundle. The caller is responsible
// for releasing the returned CFObject.
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
