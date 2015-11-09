//
//  AudioHubDeviceTests.m
//  AudioHub
//
//  Created by Daniel Lindenfelser on 09/10/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "CAHALAudioObjectTester.h"
#include "Device.h"
#include "CAPropertyAddress.h"

@interface AudioHubDeviceTests : XCTestCase
@property CAObject *object;
@property AudioObjectID objectId;
@end

@implementation AudioHubDeviceTests

- (void)setUp {
    [super setUp];
    
    _objectId = CAObjectMap::GetNextObjectID();
    _object = new Device(_objectId);
    CAObjectMap::MapObject(_objectId, _object);
    _object->Activate();
    
}

- (void)tearDown {
    _object->Deactivate();
    CAObjectMap::UnmapObject(_objectId, _object);

    [super tearDown];
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


- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
