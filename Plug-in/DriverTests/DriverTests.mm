//
//  DriverTests.m
//  DriverTests
//
//  Created by Daniel Lindenfelser on 15/06/15.
//  Copyright © 2015 ultraschall.fm. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <CoreAudio/AudioServerPlugIn.h>
#include "CAAutoDisposer.h"
#include "CAHALAudioSystemObject.h"
#include "CAHALAudioDevice.h"
#include "CAPropertyAddress.h"

@interface DriverTests : XCTestCase
@property CAHALAudioSystemObject systemObject;
@property AudioObjectID pluginObjectId;
@property CAHALAudioObject* pluginObject;
@property CAHALAudioDevice* audioDevice;
@end


//    kAudioDevicePropertyConfigurationApplication        = 'capp',
//    kAudioDevicePropertyDeviceUID                       = 'uid ',
//    kAudioDevicePropertyModelUID                        = 'muid',
//    kAudioDevicePropertyTransportType                   = 'tran',
//    kAudioDevicePropertyRelatedDevices                  = 'akin',
//    kAudioDevicePropertyClockDomain                     = 'clkd',
//    kAudioDevicePropertyDeviceIsAlive                   = 'livn',
//    kAudioDevicePropertyDeviceIsRunning                 = 'goin',
//    kAudioDevicePropertyDeviceCanBeDefaultDevice        = 'dflt',
//    kAudioDevicePropertyDeviceCanBeDefaultSystemDevice  = 'sflt',
//    kAudioDevicePropertyLatency                         = 'ltnc',
//    kAudioDevicePropertyStreams                         = 'stm#',
//    kAudioObjectPropertyControlList                     = 'ctrl',
//    kAudioDevicePropertySafetyOffset                    = 'saft',
//    kAudioDevicePropertyNominalSampleRate               = 'nsrt',
//    kAudioDevicePropertyAvailableNominalSampleRates     = 'nsr#',
//    kAudioDevicePropertyIcon                            = 'icon',
//    kAudioDevicePropertyIsHidden                        = 'hidn',
//    kAudioDevicePropertyPreferredChannelsForStereo      = 'dch2',
//    kAudioDevicePropertyPreferredChannelLayout          = 'srnd'

@implementation DriverTests

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

#pragma mark Audio Object Properties

- (void)testAudioObjectPropertyName {
    CAPropertyAddress address(kAudioObjectPropertyName);
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.audioDevice->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

#pragma mark Audio Device Properties

- (void)testAudioDevicePropertyConfigurationApplication {
    CAPropertyAddress address(kAudioDevicePropertyConfigurationApplication);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.audioDevice->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

- (void)testAudioDevicePropertyDeviceUID {
    CAPropertyAddress address(kAudioDevicePropertyDeviceUID);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.audioDevice->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

- (void)testAudioDevicePropertyModelUID {
    CAPropertyAddress address(kAudioDevicePropertyModelUID);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.audioDevice->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
}

- (void)testAudioDevicePropertyTransportType {
    
}

- (void)testAudioDevicePropertyRelatedDevices {
    
}

- (void)testAudioDevicePropertyClockDomain {
    
}

- (void)testAudioDevicePropertyDeviceIsAlive {
    
}

- (void)testAudioDevicePropertyDeviceIsRunning {
    
}

- (void)testAudioDevicePropertyDeviceCanBeDefaultDevice {
    
}

- (void)testAudioDevicePropertyDeviceCanBeDefaultSystemDevice {
    
}

- (void)testAudioDevicePropertyLatency {
    
}

- (void)testAudioDevicePropertyStreams {
    
}

- (void)testAudioObjectPropertyControlList {
    
}

- (void)testAudioDevicePropertySafetyOffset {
    
}

- (void)testAudioDevicePropertyNominalSampleRate {
    
}

- (void)testAudioDevicePropertyAvailableNominalSampleRates {
    
}

- (void)testAudioDevicePropertyIcon {
    
}

- (void)testAudioDevicePropertyIsHidden {
    
}

- (void)testAudioDevicePropertyPreferredChannelsForStereo {
    
}

- (void)testAudioDevicePropertyPreferredChannelLayout {
    
}

@end
