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

#pragma mark kAudioDevicePropertyConfigurationApplication
// A CFString that contains the bundle ID for an application that provides a
// GUI for configuring the AudioDevice. By default, the value of this property
// is the bundle ID for Audio MIDI Setup. The caller is responsible for
// releasing the returned CFObject.
- (void)testAudioDevicePropertyConfigurationApplication {
    CAPropertyAddress address(kAudioDevicePropertyConfigurationApplication);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.audioDevice->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

#pragma mark kAudioDevicePropertyDeviceUID
// A CFString that contains a persistent identifier for the AudioDevice. An
// AudioDevice's UID is persistent across boots. The content of the UID string
// is a black box and may contain information that is unique to a particular
// instance of an AudioDevice's hardware or unique to the CPU. Therefore they
// are not suitable for passing between CPUs or for identifying similar models
// of hardware. The caller is responsible for releasing the returned CFObject.
- (void)testAudioDevicePropertyDeviceUID {
    CAPropertyAddress address(kAudioDevicePropertyDeviceUID);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFStringRef data = self.audioDevice->GetPropertyData_CFString(address);
    
    NSString *result = (NSString *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertFalse([result isEqualToString:@""]);
    CFRelease(data);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
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
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyTransportType {
    CAPropertyAddress address(kAudioDevicePropertyTransportType);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    
    XCTAssertTrue(data == kAudioDeviceTransportTypeVirtual);
}

- (void)testAudioDevicePropertyRelatedDevices {
    CAPropertyAddress address(kAudioDevicePropertyRelatedDevices);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 ioNumberItems = self.audioDevice->GetPropertyData_ArraySize<AudioObjectID>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<AudioObjectID> outArray(ioNumberItems);
    self.audioDevice->GetPropertyData_Array<AudioObjectID>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyClockDomain {
    CAPropertyAddress address(kAudioDevicePropertyClockDomain);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 1337);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyDeviceIsAlive {
    CAPropertyAddress address(kAudioDevicePropertyDeviceIsAlive);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 1);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyDeviceIsRunning {
    CAPropertyAddress address(kAudioDevicePropertyDeviceIsRunning);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyDeviceCanBeDefaultDevice_InputScope {
    CAPropertyAddress address(kAudioDevicePropertyDeviceCanBeDefaultDevice, kAudioObjectPropertyScopeInput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertNotEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyDeviceCanBeDefaultDevice_OutputScope {
    CAPropertyAddress address(kAudioDevicePropertyDeviceCanBeDefaultDevice, kAudioObjectPropertyScopeOutput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertNotEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyDeviceCanBeDefaultSystemDevice {
    CAPropertyAddress address(kAudioDevicePropertyDeviceCanBeDefaultSystemDevice, kAudioObjectPropertyScopeOutput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertNotEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyLatency_InputScope {
    CAPropertyAddress address(kAudioDevicePropertyLatency, kAudioObjectPropertyScopeInput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyLatency_OutputScope {
    CAPropertyAddress address(kAudioDevicePropertyLatency, kAudioObjectPropertyScopeOutput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

#pragma mark kAudioDevicePropertyStreams
// An array of AudioStreamIDs that represent the AudioStreams of the
// AudioDevice. Note that if a notification is received for this property, any
// cached AudioStreamIDs for the device become invalid and need to be
// re-fetched.
- (void)testAudioDevicePropertyStreams {
    CAPropertyAddress address(kAudioDevicePropertyStreams);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 ioNumberItems = self.audioDevice->GetPropertyData_ArraySize<AudioObjectID>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<AudioObjectID> outArray(ioNumberItems);
    self.audioDevice->GetPropertyData_Array<AudioObjectID>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

#pragma mark kAudioObjectPropertyControlList
// An array of AudioObjectIDs that represent the AudioControls of the
// AudioDevice. Note that if a notification is received for this property, any
// cached AudioObjectIDs for the device become invalid and need to be
// re-fetched.
- (void)testAudioObjectPropertyControlList {
    CAPropertyAddress address(kAudioDevicePropertyStreams);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 ioNumberItems = self.audioDevice->GetPropertyData_ArraySize<AudioObjectID>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<AudioObjectID> outArray(ioNumberItems);
    self.audioDevice->GetPropertyData_Array<AudioObjectID>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

#pragma mark kAudioDevicePropertySafetyOffset
// A UInt32 whose value indicates the number for frames in ahead (for output)
// or behind (for input the current hardware position that is safe to do IO.
- (void)testAudioDevicePropertySafetyOffset_InputScope {
    CAPropertyAddress address(kAudioDevicePropertySafetyOffset, kAudioObjectPropertyScopeInput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertySafetyOffset_OutputScope {
    CAPropertyAddress address(kAudioDevicePropertySafetyOffset, kAudioObjectPropertyScopeOutput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 0);
    
    XCTAssertFalse(self.audioDevice->IsPropertySettable(address));
}

#pragma mark kAudioDevicePropertyNominalSampleRate
// A Float64 that indicates the current nominal sample rate of the AudioDevice.
- (void)testAudioDevicePropertyNominalSampleRate {
    CAPropertyAddress address(kAudioDevicePropertyNominalSampleRate);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    Float64 data = self.audioDevice->GetPropertyData_Float64(address);
    XCTAssertNotEqual(data, 0);
    
    XCTAssertTrue(self.audioDevice->IsPropertySettable(address));
}

- (void)testAudioDevicePropertyNominalSampleRate_Set {
    CAPropertyAddress address(kAudioDevicePropertyNominalSampleRate);
    try {
        self.audioDevice->SetPropertyData_Float64(address, 0);
        XCTAssertTrue(false);
    } catch (CAException ex) {
        self.audioDevice->SetPropertyData_Float64(address, 44100.0);
        self.audioDevice->SetPropertyData_Float64(address, 48000.0);
        self.audioDevice->SetPropertyData_Float64(address, 96000.0);
        self.audioDevice->SetPropertyData_Float64(address, 44100.0);
    }
}

#pragma mark kAudioDevicePropertyAvailableNominalSampleRates
// An array of AudioValueRange structs that indicates the valid ranges for the
// nominal sample rate of the AudioDevice.
- (void)testAudioDevicePropertyAvailableNominalSampleRates {
    CAPropertyAddress address(kAudioDevicePropertyAvailableNominalSampleRates);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 ioNumberItems = self.audioDevice->GetPropertyData_ArraySize<AudioValueRange>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<AudioValueRange> outArray(ioNumberItems);
    self.audioDevice->GetPropertyData_Array<AudioValueRange>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        Float64 max = outArray[index].mMaximum;
        Float64 min = outArray[index].mMinimum;
        XCTAssertGreaterThan(max, 0);
        XCTAssertGreaterThan(min, 0);
        XCTAssertEqual(max, min);
    }
}

#pragma mark kAudioDevicePropertyIcon
// A CFURLRef that indicates an image file that can be used to represent the
// device visually. The caller is responsible for releasing the returned
// CFObject.
- (void)testAudioDevicePropertyIcon {
    CAPropertyAddress address(kAudioDevicePropertyIcon);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    CFURLRef data;
    UInt32 ioDataSize = sizeof(CFURLRef);
    self.audioDevice->GetPropertyData(address, 0, NULL, ioDataSize, &data);
    NSURL *result = (NSURL *)CFBridgingRelease(data);
    XCTAssertNotNil(result);
    XCTAssertTrue([result isFileURL]);
}

#pragma mark kAudioDevicePropertyIsHidden
// A UInt32 where a non-zero value indicates that the device is not included
// in the normal list of devices provided by kAudioHardwarePropertyDevices nor
// can it be the default device. Hidden devices can only be discovered by
// knowing their UID and using kAudioHardwarePropertyDeviceForUID.
- (void)testAudioDevicePropertyIsHidden {
    CAPropertyAddress address(kAudioDevicePropertyIsHidden);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 dataSize = self.audioDevice->GetPropertyDataSize(address, 0, NULL);
    XCTAssertGreaterThan(dataSize, 0);
    
    UInt32 data = self.audioDevice->GetPropertyData_UInt32(address);
    XCTAssertEqual(data, 0);
}

#pragma mark kAudioDevicePropertyPreferredChannelsForStereo
// An array of two UInt32s, the first for the left channel, the second for the
// right channel, that indicate the channel numbers to use for stereo IO on the
// device. The value of this property can be different for input and output and
// there are no restrictions on the channel numbers that can be used.
- (void)testAudioDevicePropertyPreferredChannelsForStereo_InputScope {
    CAPropertyAddress address(kAudioDevicePropertyPreferredChannelsForStereo, kAudioObjectPropertyScopeInput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 ioNumberItems = self.audioDevice->GetPropertyData_ArraySize<UInt32>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<UInt32> outArray(ioNumberItems);
    self.audioDevice->GetPropertyData_Array<UInt32>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
}

- (void)testAudioDevicePropertyPreferredChannelsForStereo_OutputScope {
    CAPropertyAddress address(kAudioDevicePropertyPreferredChannelsForStereo, kAudioObjectPropertyScopeOutput);
    XCTAssertTrue(self.audioDevice->HasProperty(address));
    UInt32 ioNumberItems = self.audioDevice->GetPropertyData_ArraySize<UInt32>(address);
    XCTAssertGreaterThan(ioNumberItems, 0);
    
    CAAutoArrayDelete<UInt32> outArray(ioNumberItems);
    self.audioDevice->GetPropertyData_Array<UInt32>(address, ioNumberItems, outArray);
    
    XCTAssertGreaterThan(ioNumberItems, 0);
    for (int index = 0; index < ioNumberItems; index++) {
        XCTAssertGreaterThan(outArray[index], 0);
    }
}

#pragma mark kAudioDevicePropertyPreferredChannelLayout
// An AudioChannelLayout that indicates how each channel of the AudioDevice
// should be used.
- (void)testAudioDevicePropertyPreferredChannelLayout {
    CAPropertyAddress address(kAudioDevicePropertyPreferredChannelLayout);
    XCTAssertFalse(self.audioDevice->HasProperty(address));
}

@end
