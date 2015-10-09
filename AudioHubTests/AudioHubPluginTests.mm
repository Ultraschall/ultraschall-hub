//
//  AudioHubPluginTests.m
//  AudioHub
//
//  Created by Daniel Lindenfelser on 09/10/15.
//  Copyright © 2015 Daniel Lindenfelser. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "CAHALAudioObjectTester.h"
#include "PlugIn.h"
#include "CAPropertyAddress.h"

@interface AudioHubPluginTests : XCTestCase

@end

@implementation AudioHubPluginTests

- (void)setUp {
    [super setUp];
    PlugIn::GetInstance();
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testPropertyManufacturer {
    CAHALAudioObjectTester tester(&PlugIn::GetInstance());
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

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
