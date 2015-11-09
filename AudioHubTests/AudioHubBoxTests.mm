//
//  AudioHubBoxTests.m
//  AudioHub
//
//  Created by Daniel Lindenfelser on 23/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <CoreAudio/AudioServerPlugIn.h>
#if !ULTRASCHALL
#if !TEST
#include "AudioHubTypes.h"
#else
#include "AudioHubTestTypes.h"
#endif
#else
#if !TEST
#include "UltraschallHubTypes.h"
#else
#include "UltraschallHubTestTypes.h"
#endif
#endif
#include "CAObject.h"
#include "CAHALAudioObjectTester.h"
#include "CAPropertyAddress.h"
#include "Box.h"
#import "Settings.h"

@interface AudioHubBoxTests : XCTestCase
@property CAObject *object;
@property AudioObjectID objectId;
@end

@implementation AudioHubBoxTests

- (void)setUp {
    [super setUp];
    _objectId = CAObjectMap::GetNextObjectID();
    _object = new Box(_objectId);
    CAObjectMap::MapObject(_objectId, _object);
    _object->Activate();
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    _object->Deactivate();
    CAObjectMap::UnmapObject(_objectId, _object);
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testName {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertyName);
    XCTAssert(tester.HasProperty(address));
    XCTAssert(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    
    auto inData = CFSTR("testdata");
    tester.SetPropertyData_CFString(address, inData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    
    XCTAssert(CFStringCompare(inData, outData, kCFCompareLocalized) == kCFCompareEqualTo);
    CFRelease(outData);
}

- (void)testPropertyModelName {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertyModelName);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
}

- (void)testPropertyManufacturer {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertyManufacturer);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
}


- (void)testPropertyIdentify {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertyIdentify);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 0);
}

- (void)testPropertySerialNumber {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertySerialNumber);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
}

- (void)testPropertyFirmwareVersion {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertyFirmwareVersion);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
}

#if !ULTRASCHALL
- (void)testPropertyCustomPropertyInfoList {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioObjectPropertyCustomPropertyInfoList);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outSize = tester.GetPropertyData_ArraySize<AudioServerPlugInCustomPropertyInfo>(address);
    XCTAssert(outSize > 0);
    outSize = outSize * SizeOf32(AudioServerPlugInCustomPropertyInfo);
    AudioServerPlugInCustomPropertyInfo outData[outSize];
    tester.GetPropertyData_Array(address, outSize, outData);
    
    XCTAssert(outSize > 0);
    XCTAssert(outData[0].mSelector == kAudioHubCustomPropertySettings);
    XCTAssert(outData[0].mQualifierDataType == kAudioServerPlugInCustomPropertyDataTypeNone);
    XCTAssert(outData[0].mPropertyDataType == kAudioServerPlugInCustomPropertyDataTypeCFPropertyList);
    XCTAssert(outData[1].mSelector == kAudioHubCustomPropertyActive);
    XCTAssert(outData[1].mQualifierDataType == kAudioServerPlugInCustomPropertyDataTypeNone);
    XCTAssert(outData[1].mPropertyDataType == kAudioServerPlugInCustomPropertyDataTypeCFString);
}

- (void)testAudioHubCustomPropertySettings {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioHubCustomPropertySettings);
    XCTAssert(tester.HasProperty(address));
    XCTAssert(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFType(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFType(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);

    AudioHubSettings* settings = [[AudioHubSettings alloc] init];
    [settings addDevice:@"name" andUID:@"uid" andChannels:2];
    [settings addDevice:@"name2" andUID:@"uid2" andChannels:4];
    tester.SetPropertyData_CFType(address, (__bridge CFDictionaryRef)[settings getSettings]);
    settings = nil;
    
    tester.SetPropertyData_CFString(CAPropertyAddress(kAudioHubCustomPropertyActive), CFSTR("NO"));
    settings = [[AudioHubSettings alloc] init];
    [settings addDevice:@"name" andUID:@"uid" andChannels:2];
    [settings addDevice:@"name2" andUID:@"uid2" andChannels:4];
    tester.SetPropertyData_CFType(address, (__bridge CFDictionaryRef)[settings getSettings]);
    settings = nil;
    
    tester.SetPropertyData_CFString(CAPropertyAddress(kAudioHubCustomPropertyActive), CFSTR("YES"));
    settings = [[AudioHubSettings alloc] init];
    [settings addDevice:@"name" andUID:@"uid" andChannels:2];
    [settings addDevice:@"name2" andUID:@"uid2" andChannels:4];
    tester.SetPropertyData_CFType(address, (__bridge CFDictionaryRef)[settings getSettings]);
    settings = nil;
    
    outData = tester.GetPropertyData_CFType(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    
    outData = tester.GetPropertyData_CFType(address);
    XCTAssert(outData != NULL);
    settings = [[AudioHubSettings alloc] initWithSettings:(__bridge NSDictionary*)outData];
    CFRelease(outData);
    XCTAssert(settings.numberOfDevices > 0);
}

- (void)testAudioHubCustomPropertyActive {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioHubCustomPropertyActive);
    CAPropertyAddress testAddress(kAudioBoxPropertyAcquired);
    XCTAssert(tester.HasProperty(address));
    XCTAssert(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    
    auto inData = CFSTR("NO");
    tester.SetPropertyData_CFString(address, inData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    
    XCTAssert(CFStringCompare(inData, outData, kCFCompareLocalized) == kCFCompareEqualTo);
    CFRelease(outData);
    XCTAssertFalse(tester.GetPropertyData_UInt32(testAddress));
    
    inData = CFSTR("YES");
    tester.SetPropertyData_CFString(address, inData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    
    XCTAssert(CFStringCompare(inData, outData, kCFCompareLocalized) == kCFCompareEqualTo);
    CFRelease(outData);
    XCTAssert(tester.GetPropertyData_UInt32(testAddress));
}
#endif

- (void)testPropertyBoxUID {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyBoxUID);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
    outData = tester.GetPropertyData_CFString(address);
    XCTAssert(outData != NULL);
    CFRelease(outData);
}

- (void)testPropertyTransportType {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyTransportType);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == kAudioDeviceTransportTypeVirtual);
}

- (void)testPropertyHasAudio {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyHasAudio);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 1);
}

- (void)testPropertyHasVideo {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyHasVideo);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 0);
}

- (void)testPropertyHasMIDI {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyHasMIDI);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 0);
}

- (void)testPropertyIsProtected {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyIsProtected);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 0);
}

#if !ULTRASCHALL
- (void)testPropertyAcquired {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyAcquired);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 1);

    tester.SetPropertyData_CFString(CAPropertyAddress(kAudioHubCustomPropertyActive), CFSTR("NO"));
    outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 0);
    
    tester.SetPropertyData_CFString(CAPropertyAddress(kAudioHubCustomPropertyActive), CFSTR("YES"));
    outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 1);
}
#endif

- (void)testPropertyAcquisitionFailed {
    CAHALAudioObjectTester tester(_object);
    tester.GetObjectID();
    CAPropertyAddress address(kAudioBoxPropertyAcquisitionFailed);
    XCTAssert(tester.HasProperty(address));
    XCTAssertFalse(tester.IsPropertySettable(address));
    
    auto outData = tester.GetPropertyData_UInt32(address);
    XCTAssert(outData == 0);
}

@end
