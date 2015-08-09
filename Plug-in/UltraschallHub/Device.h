//
//  Driver.h
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 02/11/14.
//  Copyright (c) 2014 ultraschall.fm. All rights reserved.
//

#ifndef __UltraschallHub__Driver__
#define __UltraschallHub__Driver__

#include <CoreAudio/AudioServerPlugIn.h>
#include <CoreAudio/AudioHardwareBase.h>
#include <CoreAudio/CoreAudioTypes.h>

#include "PlugIn.h"

#include "CACFString.h"
#include "CAMutex.h"
#include "CAVolumeCurve.h"
#include "CAObject.h"
#include "CARingBuffer.h"
#include "CAHostTimeBase.h"
#include "CAStreamRangedDescription.h"

//	volume control ranges
#define kUltraschallHub_Control_MinRawVolumeValue 0
#define kUltraschallHub_Control_MaxRawVolumeValue 96
#define kUltraschallHub_Control_MinDBVolumeValue -96.0f
#define kUltraschallHub_Control_MaxDbVolumeValue 0.0f

#define kUltraschallHub_StreamFormatChange 1
#define kUltraschallHub_SampleRateChange 2

//	the struct in the status buffer
struct SimpleAudioDriverStatus {
    volatile UInt64 mSampleTime;
    volatile UInt64 mHostTime;
};
typedef struct SimpleAudioDriverStatus SimpleAudioDriverStatus;

class UltHub_Device : public CAObject {
#pragma mark Construction/Destruction
public:
    UltHub_Device(AudioObjectID inObjectID, SInt16 numChannels = 2);

    virtual void Activate();

    virtual void Deactivate();

protected:
    virtual ~UltHub_Device();

#pragma mark Property Operations
public:
    virtual bool HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    virtual bool IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    virtual UInt32 GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const;

    virtual void GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const;

    virtual void SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);

#pragma mark Device Property Operations
private:
    bool Device_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    bool Device_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    UInt32 Device_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const;

    void Device_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const;

    void Device_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);

#pragma mark Stream Property Operations
private:
    bool Stream_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    bool Stream_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    UInt32 Stream_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const;

    void Stream_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const;

    void Stream_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);

#pragma mark Control Property Operations
private:
    bool Control_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    bool Control_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;

    UInt32 Control_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const;

    void Control_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const;

    void Control_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);

#pragma mark IO Operations
public:
    void StartIO();

    void StopIO();

    void GetZeroTimeStamp(Float64& outSampleTime, UInt64& outHostTime, UInt64& outSeed) const;

    void WillDoIOOperation(UInt32 inOperationID, bool& outWillDo, bool& outWillDoInPlace) const;

    void BeginIOOperation(UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo& inIOCycleInfo);

    void DoIOOperation(AudioObjectID inStreamObjectID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo& inIOCycleInfo, void* ioMainBuffer, void* ioSecondaryBuffer);

    void EndIOOperation(UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo& inIOCycleInfo);

private:
    void ReadInputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void* outBuffer);

    void WriteOutputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void* inBuffer);

#pragma mark Implementation
public:
    CFStringRef CopyDeviceUID() const
    {
        return mDeviceUID.CopyCFString();
    }

    void PerformConfigChange(UInt64 inChangeAction, void* inChangeInfo);

    void AbortConfigChange(UInt64 inChangeAction, void* inChangeInfo);

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

    CAMutex* mStateMutex;
    CAMutex* mIOMutex;

    CACFString mDeviceUID;
    CACFString mDeviceName;

public:
    void setDeviceUID(CFStringRef uid) { this->mDeviceUID = uid; }
    void setDeviceName(CFStringRef name) { this->mDeviceName = name; }
    CFStringRef getDeviceUID() { return this->mDeviceUID.GetCFString(); }

private:
    // IO
    UInt64 mStartCount;

    // Timing / Clock
    Float64 mTicksPerFrame;
    UInt64 mAnchorHostTime;
    UInt64 mTimeline;

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
    
    const int mSafetyOffsetInput = 64;
    const int mSafetyOffsetOutput = 0;
    const int mLatencyInput = 1;
    const int mLatencyOutput = 0;
};

#endif /* defined(__UltraschallHub__Driver__) */
