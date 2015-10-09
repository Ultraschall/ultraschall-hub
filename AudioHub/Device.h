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

#ifndef __Driver__
#define __Driver__


#include "CACFString.h"
#include "CAMutex.h"
#include "CAVolumeCurve.h"
#include "CAObject.h"
#include "CARingBuffer.h"
#include "CAHostTimeBase.h"
#include "CAStreamRangedDescription.h"

//	volume control ranges
#define kHub_Control_MinRawVolumeValue 0
#define kHub_Control_MaxRawVolumeValue 96
#define kHub_Control_MinDBVolumeValue -96.0f
#define kHub_Control_MaxDbVolumeValue 0.0f

#define kHub_StreamFormatChange 1
#define kHub_SampleRateChange 2

//	the struct in the status buffer
struct SimpleAudioDriverStatus {
    volatile UInt64 mSampleTime;
    volatile UInt64 mHostTime;
};
typedef struct SimpleAudioDriverStatus SimpleAudioDriverStatus;

class Device : public CAObject {
#pragma mark Construction/Destruction
public:
    Device(AudioObjectID inObjectID, SInt16 numChannels = 2, AudioObjectID owner = kAudioObjectPlugInObject);
    virtual void Activate();
    virtual void Deactivate();

protected:
    virtual ~Device();

#pragma mark Property Operations
public:
    virtual bool HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    virtual bool IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    virtual UInt32 GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const;
    virtual void GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const;
    virtual void SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

#pragma mark Device Property Operations
private:
    bool Device_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    bool Device_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    UInt32 Device_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const;
    void Device_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const;
    void Device_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

#pragma mark Stream Property Operations
private:
    bool Stream_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    bool Stream_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    UInt32 Stream_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const;
    void Stream_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const;
    void Stream_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

#pragma mark Control Property Operations
private:
    bool Control_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    bool Control_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    UInt32 Control_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const;
    void Control_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const;
    void Control_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

#pragma mark IO Operations
public:
    void StartIO();
    void StopIO();
    void ResetIO();
    void GetZeroTimeStamp(Float64 &outSampleTime, UInt64 &outHostTime, UInt64 &outSeed);
    void WillDoIOOperation(UInt32 inOperationID, bool &outWillDo, bool &outWillDoInPlace) const;
    void BeginIOOperation(UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo &inIOCycleInfo);
    void DoIOOperation(AudioObjectID inStreamObjectID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo &inIOCycleInfo, void *ioMainBuffer, void *ioSecondaryBuffer);
    void EndIOOperation(UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo &inIOCycleInfo);

private:
    void ReadInputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void *outBuffer);
    void WriteOutputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void *inBuffer);

#pragma mark Implementation
public:
    CFStringRef CopyDeviceUID() const {
        return mDeviceUID.CopyCFString();
    }

    void PerformConfigChange(UInt64 inChangeAction, void *inChangeInfo);
    void AbortConfigChange(UInt64 inChangeAction, void *inChangeInfo);

private:
    enum {
        kNumberOfSubObjects = 4,
        kNumberOfInputSubObjects = 2,
        kNumberOfOutputSubObjects = 2,

        kNumberOfStreams = 2,
        kNumberOfInputStreams = 1,
        kNumberOfOutputStreams = 1,

        kNumberOfControls = 2
    };

    CAMutex *mStateMutex;
    CAMutex *mIOMutex;

    CACFString mDeviceUID;
    CACFString mDeviceName;

public:
    void setDeviceUID(CFStringRef uid) {
        this->mDeviceUID = uid;
    }

    void setDeviceName(CFStringRef name) {
        this->mDeviceName = name;
    }

    CFStringRef getDeviceUID() {
        return this->mDeviceUID.GetCFString();
    }
    
    CFStringRef GetDeviceName() {
        return this->mDeviceName.GetCFString();
    }

    UInt32 GetChannels() {
        return this->mStreamDescription.mChannelsPerFrame;
    }
private:
    // IO
    UInt64 mStartCount;

    // Timing / Clock
    Float64 mTicksPerFrame;
    UInt64 mAnchorHostTime;
    UInt64 mTimeline;
    UInt64 mNumberTimeStamps = 0;
    UInt64 mCurrentTimeLine = 0;

    // Audio Ring Buffer
    CARingBuffer mRingBuffer;
    UInt32 mRingBufferSize;

    // Steam
    typedef std::vector<CAStreamBasicDescription> StreamDescriptionList;
    StreamDescriptionList mStreamDescriptions;
    AudioStreamBasicDescription mStreamDescription;

    AudioObjectID mInputStreamObjectID;
    AudioObjectID mOutputStreamObjectID;
    bool mOutputStreamIsActive;
    bool mInputStreamIsActive;

    // Controls
    AudioObjectID mInputMasterVolumeControlObjectID;
    SInt32 mInputMasterVolumeControlRawValueShadow;
    AudioObjectID mOutputMasterVolumeControlObjectID;
    SInt32 mOutputMasterVolumeControlRawValueShadow;
    CAVolumeCurve mVolumeCurve;
    Float32 mMasterInputVolume;
    Float32 mMasterOutputVolume;

public:
    enum class Offset : UInt32 {
        Stable = 256,
        Normal = 128,
        Fast = 64,
        Faster = 32,
        Insane = 16,
        Off = 0
    };
    UInt32 offset_to_uint32 (Offset offset) const {
        return static_cast<UInt32>(offset);
    }
private:
    // IO
    Offset mSafetyOffsetInput = Offset::Off;
    Offset mSafetyOffsetOutput = Offset::Normal;
    const int mLatencyInput = 0;
    const int mLatencyOutput = 0;

    UInt32 isHidden = 0;
};

#endif /* defined(__Driver__) */
