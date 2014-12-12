//
// Created by Daniel Lindenfelser on 04/11/14.
// Copyright (c) 2014 ultraschall.fm. All rights reserved.
//

#ifndef __UltraschallHub_Factory__
#define __UltraschallHub_Factory__

#include <CoreAudio/AudioServerPlugIn.h>

//	Entry points for the COM methods
extern "C"

    void*
    UltHub_Create(CFAllocatorRef inAllocator, CFUUIDRef inRequestedTypeUUID);

static HRESULT UltHub_QueryInterface(void* inDriver, REFIID inUUID, LPVOID* outInterface);

static ULONG UltHub_AddRef(void* inDriver);

static ULONG UltHub_Release(void* inDriver);

static OSStatus UltHub_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost);

static OSStatus UltHub_CreateDevice(AudioServerPlugInDriverRef inDriver, CFDictionaryRef inDescription, const AudioServerPlugInClientInfo* inClientInfo, AudioObjectID* outDeviceObjectID);

static OSStatus UltHub_DestroyDevice(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID);

static OSStatus UltHub_AddDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo* inClientInfo);

static OSStatus UltHub_RemoveDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo* inClientInfo);

static OSStatus UltHub_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo);

static OSStatus UltHub_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo);

static Boolean UltHub_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress);

static OSStatus UltHub_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable);

static OSStatus UltHub_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* outDataSize);

static OSStatus UltHub_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32* outDataSize, void* outData);

static OSStatus UltHub_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);

static OSStatus UltHub_StartIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID);

static OSStatus UltHub_StopIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID);

static OSStatus UltHub_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, Float64* outSampleTime, UInt64* outHostTime, UInt64* outSeed);

static OSStatus UltHub_WillDoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, Boolean* outWillDo, Boolean* outWillDoInPlace);

static OSStatus UltHub_BeginIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo* inIOCycleInfo);

static OSStatus UltHub_DoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, AudioObjectID inStreamObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo* inIOCycleInfo, void* ioMainBuffer, void* ioSecondaryBuffer);

static OSStatus UltHub_EndIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo* inIOCycleInfo);

#pragma mark The COM Interface

static AudioServerPlugInDriverInterface gAudioServerPlugInDriverInterface = {
    NULL,
    UltHub_QueryInterface,
    UltHub_AddRef,
    UltHub_Release,
    UltHub_Initialize,
    UltHub_CreateDevice,
    UltHub_DestroyDevice,
    UltHub_AddDeviceClient,
    UltHub_RemoveDeviceClient,
    UltHub_PerformDeviceConfigurationChange,
    UltHub_AbortDeviceConfigurationChange,
    UltHub_HasProperty,
    UltHub_IsPropertySettable,
    UltHub_GetPropertyDataSize,
    UltHub_GetPropertyData,
    UltHub_SetPropertyData,
    UltHub_StartIO,
    UltHub_StopIO,
    UltHub_GetZeroTimeStamp,
    UltHub_WillDoIOOperation,
    UltHub_BeginIOOperation,
    UltHub_DoIOOperation,
    UltHub_EndIOOperation
};

#endif //__UltraschallHub_Factory__
