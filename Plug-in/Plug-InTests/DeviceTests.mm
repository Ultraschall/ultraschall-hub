//
//  DeviceTests.m
//  UltraschallHubDriver
//
//  Created by Daniel Lindenfelser on 09/08/15.
//  Copyright © 2015 ultraschall.fm. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "Device.h"

@interface DeviceTests : XCTestCase

@end

@implementation DeviceTests

UInt32 deviceObjectID = -1;

- (void)setUp {
    [super setUp];

    deviceObjectID = CAObjectMap::GetNextObjectID();
    auto device = new UltHub_Device(deviceObjectID);
    CAObjectMap::RetainObject(device);
    CAObjectMap::MapObject(deviceObjectID, device);
    device->Activate();
    
    CAObjectMap::Dump();
}

- (void)tearDown {
    CAObjectReleaser<UltHub_Device> theDeadDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(deviceObjectID));
    CAObjectMap::UnmapObject(deviceObjectID, theDeadDevice);
    [super tearDown];
}

- (void)testInitTest {
    CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(deviceObjectID));
    XCTAssert(theDevice.IsValid(), "unknown device");
}

- (void)testPerformanceWrite {
    CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(deviceObjectID));
    XCTAssert(theDevice.IsValid(), "unknown device");


    auto ioBufferFrameSize = 512;
    auto defaultStreamDescription = CAStreamBasicDescription(48000, 2, CAStreamBasicDescription::kPCMFormatFloat32, true);
    auto bufferSize = (defaultStreamDescription.mChannelsPerFrame * defaultStreamDescription.mBytesPerFrame) * ioBufferFrameSize;
    auto buffer = malloc(bufferSize);
    memset(buffer, 0, bufferSize);

    auto audioServerPlugInIOCycleInfo = AudioServerPlugInIOCycleInfo();
    audioServerPlugInIOCycleInfo.mInputTime.mSampleTime = 0;
    
    theDevice->StartIO();
    [self measureBlock:^{
        for (auto i = 0; i < 1000000; i++) {
            theDevice->DoIOOperation(-1, kAudioServerPlugInIOOperationWriteMix, ioBufferFrameSize, audioServerPlugInIOCycleInfo, buffer, nullptr);
        }
    }];
    theDevice->StartIO();
    free(buffer);
}

- (void)testPerformanceRead {
    CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(deviceObjectID));
    XCTAssert(theDevice.IsValid(), "unknown device");
    
    
    auto ioBufferFrameSize = 512;
    auto defaultStreamDescription = CAStreamBasicDescription(48000, 2, CAStreamBasicDescription::kPCMFormatFloat32, true);
    auto bufferSize = (defaultStreamDescription.mChannelsPerFrame * defaultStreamDescription.mBytesPerFrame) * ioBufferFrameSize;
    auto buffer = malloc(bufferSize);
    memset(buffer, 0, bufferSize);

    
    auto audioServerPlugInIOCycleInfo = AudioServerPlugInIOCycleInfo();
    audioServerPlugInIOCycleInfo.mOutputTime.mSampleTime = 0;
    audioServerPlugInIOCycleInfo.mInputTime.mSampleTime = 0;
    
    theDevice->StartIO();
    theDevice->DoIOOperation(-1, kAudioServerPlugInIOOperationWriteMix, ioBufferFrameSize, audioServerPlugInIOCycleInfo, buffer, nullptr);
    [self measureBlock:^{
        for (auto i = 0; i < 1000000; i++) {
            theDevice->DoIOOperation(-1, kAudioServerPlugInIOOperationReadInput, ioBufferFrameSize, audioServerPlugInIOCycleInfo, buffer, nullptr);
        }
    }];
    theDevice->StartIO();
    free(buffer);
}

@end
