//
//  IOTests.m
//  UltraschallHubDriver
//
//  Created by Daniel Lindenfelser on 18/06/15.
//  Copyright © 2015 ultraschall.fm. All rights reserved.
//

#import <XCTest/XCTest.h>

#include <CoreAudio/AudioServerPlugIn.h>
#include "CAAutoDisposer.h"
#include "CAHALAudioSystemObject.h"
#include "CAHALAudioDevice.h"
#include "CAPropertyAddress.h"

@interface IOTests : XCTestCase
@property CAHALAudioSystemObject systemObject;
@property AudioObjectID pluginObjectId;
@property CAHALAudioObject* pluginObject;
@property CAHALAudioDevice* audioDevice;
@end

@implementation IOTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    self.pluginObjectId = self.systemObject.GetAudioPlugInForBundleID(CFSTR("fm.ultraschall.audio.UltraschallHub"));
    self.pluginObject = new CAHALAudioObject(self.pluginObjectId);
    
    CAPropertyAddress address(kAudioPlugInPropertyDeviceList);
    UInt32 dataSize = self.pluginObject->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 outDataSize = dataSize;
    CAAutoArrayDelete<AudioObjectID> outData(dataSize / sizeof(AudioObjectID));
    self.pluginObject->GetPropertyData(address, 0, NULL, outDataSize, outData);
    XCTAssertGreaterThan(outDataSize, 0);
    
    AudioObjectID deviceID = outData[0];
    XCTAssertGreaterThan(deviceID, 0);
    
    self.audioDevice = new CAHALAudioDevice(deviceID);
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

//- (void)testExample {
//    // This is an example of a functional test case.
//    // Use XCTAssert and related functions to verify your tests produce the correct results.
//    
//    dispatch_queue_t queue = dispatch_queue_create("fm.ultraschall.audio.UltraschallHubTest", NULL);
//
//    auto ioProcId = self.audioDevice->CreateIOProcIDWithBlock(queue, ^(const AudioTimeStamp * __nonnull inNow, const AudioBufferList * __nonnull inInputData, const AudioTimeStamp * __nonnull inInputTime, AudioBufferList * __nonnull outOutputData, const AudioTimeStamp * __nonnull inOutputTime) {
//        // test
//    });
//    self.audioDevice->StartIOProc(ioProcId);
//}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
