//
//  HubAudioTests.m
//  HubAudioTests
//
//  Created by Daniel Lindenfelser on 11/09/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "DeviceList.h"
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
#include "CACFDictionary.h"
#include "CACFArray.h"

@interface AudioHubDeviceListTests : XCTestCase

@end

@implementation AudioHubDeviceListTests

std::shared_ptr<DeviceList> deviceList;

- (void)setUp {
    [super setUp];
    deviceList = std::make_shared<DeviceList>();
}

- (void)tearDown {
    deviceList = nullptr;
    [super tearDown];
}

- (void)testAddDevice {
    auto theDevice = new Device(CAObjectMap::GetNextObjectID(), 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(CFSTR("UID"));
    deviceList->AddDevice(theDevice);
}

- (void)testRemoveDevice {
    auto theDevice = new Device(CAObjectMap::GetNextObjectID(), 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(CFSTR("UID"));
    deviceList->AddDevice(theDevice);
    deviceList->RemoveDevice(theDevice);
}

- (void)testGetDeviceObjectID {
    auto objectId = CAObjectMap::GetNextObjectID();
    auto theDevice = new Device(objectId, 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(CFSTR("UID"));
    deviceList->AddDevice(theDevice);
    auto testObjectId = deviceList->GetDeviceObjectID(0);
    XCTAssertEqual(objectId, testObjectId);
}

- (void)testGetDeviceObjectIDInvalideID {
    auto objectId = CAObjectMap::GetNextObjectID();
    auto theDevice = new Device(objectId, 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(CFSTR("UID"));
    deviceList->AddDevice(theDevice);
    auto testObjectId = deviceList->GetDeviceObjectID(1223456789);
    XCTAssertNotEqual(objectId, testObjectId);
    XCTAssertEqual(testObjectId, kAudioObjectUnknown);
}

- (void)testRemoveAllDevices {
    auto theDevice = new Device(CAObjectMap::GetNextObjectID(), 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(CFSTR("UID"));
    deviceList->AddDevice(theDevice);
    deviceList->RemoveAllDevices();
    XCTAssert(deviceList->NumDevices() == 0);
}

- (void)testGetDeviceObjectIDByUUID {
    auto objectId = CAObjectMap::GetNextObjectID();
    auto uid = CFSTR("UID");
    auto theDevice = new Device(objectId, 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(uid);
    deviceList->AddDevice(theDevice);
    auto testObjectId = deviceList->GetDeviceObjectIDByUUID(uid);
    XCTAssertEqual(objectId, testObjectId);
}

- (void)testGetDeviceObjectIDByUUIDInvalide {
    auto objectId = CAObjectMap::GetNextObjectID();
    auto uid = CFSTR("UID");
    auto theDevice = new Device(objectId, 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(uid);
    deviceList->AddDevice(theDevice);
    auto testObjectId = deviceList->GetDeviceObjectIDByUUID(CFSTR("UID123"));
    XCTAssertNotEqual(objectId, testObjectId);
    XCTAssertEqual(testObjectId, kAudioObjectUnknown);
}

- (void)testGetSettings {
    auto objectId = CAObjectMap::GetNextObjectID();
    auto uid = CFSTR("UID");
    auto theDevice = new Device(objectId, 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(uid);
    deviceList->AddDevice(theDevice);
    NSDictionary *dic = (__bridge NSDictionary*)deviceList->GetSettings();
    XCTAssertNotNil(dic);
    XCTAssertEqual([dic count], 1);
}

- (void)testSetSettings {
    auto objectId = CAObjectMap::GetNextObjectID();
    auto uid = CFSTR("UID");
    auto theDevice = new Device(objectId, 2);
    theDevice->setDeviceName(CFSTR("name"));
    theDevice->setDeviceUID(uid);
    deviceList->AddDevice(theDevice);
}

- (void)testAddDevicePropertyList {
    CACFDictionary settings;
    CACFArray devices;
    CACFDictionary device;
    
    device.AddCFType(kAudioHubSettingsKeyDeviceName, CFSTR("name"));
    device.AddCFType(kAudioHubSettingsKeyDeviceUID, CFSTR("uid"));
    device.AddUInt32(kAudioHubSettingsKeyDeviceChannels, 2);
    
    devices.AppendDictionary(device.CopyCFDictionary());
    settings.AddCFType(kAudioHubSettingsKeyDevices, devices.CopyCFArray());
    
    auto result = deviceList->SetSettings(settings.CopyCFDictionary());
    XCTAssert(result);
}

@end
