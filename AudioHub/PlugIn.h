/*
The MIT License (MIT)

Copyright (c) 2015 Daniel Lindenfelser

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __PlugIn__
#define __PlugIn__

#include <vector>
#include <memory>

#include <CoreAudio/AudioServerPlugIn.h>
#include <CoreAudio/AudioHardwareBase.h>
#include <CoreAudio/CoreAudioTypes.h>

#include "AudioHubTypes.h"

#include "CAObject.h"
#include "CAMutex.h"

#include "CACFDictionary.h"
#include "Box.h"

class Device;

class PlugIn : public CAObject {
public:
    static PlugIn &GetInstance();

protected:
    PlugIn();
    virtual ~PlugIn();
    virtual void Activate();
    virtual void Deactivate();

private:
    static void StaticInitializer();

#pragma mark Property Operations
public:
    virtual bool HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    virtual bool IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    virtual UInt32 GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const;
    virtual void GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const;
    virtual void SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

#pragma mark Box
    AudioObjectID mBoxAudioObjectID;
    Box *mBox;

#pragma mark Settings
public:
    void StoreSettings(CFPropertyListRef settings);
    void StoreSettings();
    void RestoreSettings();

#pragma mark Host Accesss
public:
    static void SetHost(AudioServerPlugInHostRef inHost) {
        sHost = inHost;
    }

    static void Host_PropertiesChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[]) {
        if (sHost != NULL)
        {
            sHost->PropertiesChanged(GetInstance().sHost, inObjectID, inNumberAddresses, inAddresses);
        }
    }

    static void Host_RequestDeviceConfigurationChange(AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
        if (sHost != NULL)
        {
            sHost->RequestDeviceConfigurationChange(GetInstance().sHost, inDeviceObjectID, inChangeAction, inChangeInfo);
        }
    }

#pragma mark Implementation
private:
    CAMutex *mMutex;

    static pthread_once_t sStaticInitializer;
    static PlugIn *sInstance;
    static AudioServerPlugInHostRef sHost;
public:
    CFStringRef bundleIdentifier = kAudioHubBundleIdentifier;
};

#endif /* defined(__PlugIn__) */
