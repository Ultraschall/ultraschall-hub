//
//  PlugIn.h
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 02/11/14.
//  Copyright (c) 2014 ultraschall.fm. All rights reserved.
//

#ifndef __UltraschallHub__PlugIn__
#define __UltraschallHub__PlugIn__

#include <vector>

#define CoreAudio_StopOnFailure 1
#define CoreAudio_TimeStampMessages 1
#define CoreAudio_ThreadStampMessages 1
#define CoreAudio_FlushDebugMessages 1

#include <CoreAudio/AudioServerPlugIn.h>
#include <CoreAudio/AudioHardwareBase.h>

#include "CAObject.h"
#include "CAMutex.h"

class UltHub_Device;

#define kUltraschallHub_BundleID "fm.ultraschall.audio.UltraschallHub"
#define kUltraschallHub_Manufacturer "fm.ultraschall"

enum {
    kAudioPlugInPropertyUltraschallSettings = 'ults',
    kAudioPlugInPropertyUltraschallDump = 'ultd',
    kAudioPlugInPropertyUltraschallNumberOfObjects = 2
};

class UltHub_PlugIn : public CAObject {
public:
    static UltHub_PlugIn& GetInstance();

protected:
    UltHub_PlugIn();

    virtual ~UltHub_PlugIn();

    virtual void Activate();

    virtual void Deactivate();

private:
    static void StaticInitializer();

#pragma mark Settings
    bool ValidateSettings(CFPropertyListRef propertyListRef);
    bool WriteSettings();
    CFPropertyListRef ReadSettings() const;

    CFPropertyListRef mCurrentSettings;

#pragma mark Property Operations
public:
    virtual bool HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    virtual bool IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    virtual UInt32 GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const;

    virtual void GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const;

    virtual void SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);

#pragma mark Device List Management
private:
    void InitializeDevices();

    void AddDevice(UltHub_Device* inDevice);

    void RemoveDevice(UltHub_Device* inDevice);

    void _AddDevice(UltHub_Device* inDevice);

    void _RemoveDevice(UltHub_Device* inDevice);

    void _RemoveAllDevices();

    struct DeviceInfo {
        AudioObjectID mDeviceObjectID;

        DeviceInfo()
            : mDeviceObjectID(0)
        {
        }

        DeviceInfo(AudioObjectID inDeviceObjectID)
            : mDeviceObjectID(inDeviceObjectID)
        {
        }
    };

    typedef std::vector<DeviceInfo> DeviceInfoList;
    DeviceInfoList mDeviceInfoList;

#pragma mark Host Accesss
public:
    static void SetHost(AudioServerPlugInHostRef inHost)
    {
        sHost = inHost;
    }

    static void Host_PropertiesChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[])
    {
        if (sHost != NULL) {
            sHost->PropertiesChanged(GetInstance().sHost, inObjectID, inNumberAddresses, inAddresses);
        }
    }

    static void Host_RequestDeviceConfigurationChange(AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo)
    {
        if (sHost != NULL) {
            sHost->RequestDeviceConfigurationChange(GetInstance().sHost, inDeviceObjectID, inChangeAction, inChangeInfo);
        }
    }

#pragma mark Implementation
private:
    CAMutex* mMutex;

    static pthread_once_t sStaticInitializer;
    static UltHub_PlugIn* sInstance;
    static AudioServerPlugInHostRef sHost;
};

#endif /* defined(__UltraschallHub__PlugIn__) */
