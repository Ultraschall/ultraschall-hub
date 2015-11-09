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

#include "Factory.h"

#include "PlugIn.h"
#include "Device.h"

#include "CACFObject.h"
#include "CAException.h"

static HRESULT AudioHub_QueryInterface(void *inDriver, REFIID inUUID, LPVOID *outInterface);
static ULONG AudioHub_AddRef(void *inDriver);
static ULONG AudioHub_Release(void *inDriver);
static OSStatus AudioHub_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost);
static OSStatus AudioHub_CreateDevice(AudioServerPlugInDriverRef inDriver, CFDictionaryRef inDescription, const AudioServerPlugInClientInfo *inClientInfo, AudioObjectID *outDeviceObjectID);
static OSStatus AudioHub_DestroyDevice(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID);
static OSStatus AudioHub_AddDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo *inClientInfo);
static OSStatus AudioHub_RemoveDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo *inClientInfo);
static OSStatus AudioHub_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo);
static OSStatus AudioHub_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo);
static Boolean AudioHub_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress);
static OSStatus AudioHub_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable);
static OSStatus AudioHub_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *outDataSize);
static OSStatus AudioHub_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 *outDataSize, void *outData);
static OSStatus AudioHub_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);
static OSStatus AudioHub_StartIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID);
static OSStatus AudioHub_StopIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID);
static OSStatus AudioHub_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, Float64 *outSampleTime, UInt64 *outHostTime, UInt64 *outSeed);
static OSStatus AudioHub_WillDoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, Boolean *outWillDo, Boolean *outWillDoInPlace);
static OSStatus AudioHub_BeginIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo);
static OSStatus AudioHub_DoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, AudioObjectID inStreamObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo, void *ioMainBuffer, void *ioSecondaryBuffer);
static OSStatus AudioHub_EndIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo);

#pragma mark The COM Interface
static AudioServerPlugInDriverInterface gAudioServerPlugInDriverInterface = {
    NULL,
    AudioHub_QueryInterface,
    AudioHub_AddRef,
    AudioHub_Release,
    AudioHub_Initialize,
    AudioHub_CreateDevice,
    AudioHub_DestroyDevice,
    AudioHub_AddDeviceClient,
    AudioHub_RemoveDeviceClient,
    AudioHub_PerformDeviceConfigurationChange,
    AudioHub_AbortDeviceConfigurationChange,
    AudioHub_HasProperty,
    AudioHub_IsPropertySettable,
    AudioHub_GetPropertyDataSize,
    AudioHub_GetPropertyData,
    AudioHub_SetPropertyData,
    AudioHub_StartIO,
    AudioHub_StopIO,
    AudioHub_GetZeroTimeStamp,
    AudioHub_WillDoIOOperation,
    AudioHub_BeginIOOperation,
    AudioHub_DoIOOperation,
    AudioHub_EndIOOperation
};

static AudioServerPlugInDriverInterface *gAudioServerPlugInDriverInterfacePtr = &gAudioServerPlugInDriverInterface;
static AudioServerPlugInDriverRef gAudioServerPlugInDriverRef = &gAudioServerPlugInDriverInterfacePtr;
static UInt32 gAudioServerPlugInDriverRefCount = 1;

#pragma mark Factory

extern "C" void* AudioHub_Create(CFAllocatorRef /*inAllocator*/, CFUUIDRef inRequestedTypeUUID) {
    void *theAnswer = NULL;
    if (CFEqual(inRequestedTypeUUID, kAudioServerPlugInTypeUUID)) {
        theAnswer = gAudioServerPlugInDriverRef;
        PlugIn::GetInstance();
    }
    return theAnswer;
}

#pragma mark Inheritence

static HRESULT AudioHub_QueryInterface(void *inDriver, REFIID inUUID, LPVOID *outInterface) {
    HRESULT theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_QueryInterface: bad driver reference");
        ThrowIfNULL(outInterface, CAException(kAudioHardwareIllegalOperationError), "AudioHub_QueryInterface: no place to store the returned interface");

        CACFUUID theRequestedUUID(CFUUIDCreateFromUUIDBytes(NULL, inUUID));
        ThrowIf(!theRequestedUUID.IsValid(), CAException(kAudioHardwareIllegalOperationError), "AudioHub_QueryInterface: failed to create the CFUUIDRef");
        ThrowIf(!CFEqual(theRequestedUUID.GetCFObject(), IUnknownUUID) && !CFEqual(theRequestedUUID.GetCFObject(), kAudioServerPlugInDriverInterfaceUUID), CAException(E_NOINTERFACE), "AudioHub_QueryInterface: requested interface is unsupported");
        ThrowIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, CAException(E_NOINTERFACE), "AudioHub_QueryInterface: the ref count is maxxed out");
        ++gAudioServerPlugInDriverRefCount;
        *outInterface = gAudioServerPlugInDriverRef;
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static ULONG AudioHub_AddRef(void *inDriver) {
    ULONG theAnswer = 0;
    FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "AudioHub_AddRef: bad driver reference");
    FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "AudioHub_AddRef: out of references");
    ++gAudioServerPlugInDriverRefCount;
    theAnswer = gAudioServerPlugInDriverRefCount;
    Done:
    return theAnswer;
}

static ULONG AudioHub_Release(void *inDriver) {
    ULONG theAnswer = 0;
    FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "AudioHub_Release: bad driver reference");
    FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "AudioHub_Release: out of references");
    --gAudioServerPlugInDriverRefCount;
    theAnswer = gAudioServerPlugInDriverRefCount;
    Done:
    return theAnswer;
}

#pragma mark Basic Operations

static OSStatus AudioHub_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_Initialize: bad driver reference");
        PlugIn::GetInstance().SetHost(inHost);
        PlugIn::GetInstance().RestoreSettings();
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_CreateDevice(AudioServerPlugInDriverRef /*inDriver*/, CFDictionaryRef /*inDescription*/, const AudioServerPlugInClientInfo * /*inClientInfo*/, AudioObjectID * /*outDeviceObjectID*/) {
    return kAudioHardwareUnsupportedOperationError;
}

static OSStatus AudioHub_DestroyDevice(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/) {
    return kAudioHardwareUnsupportedOperationError;
}

static OSStatus AudioHub_AddDeviceClient(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/, const AudioServerPlugInClientInfo * /*inClientInfo*/) {
    return 0;
}

static OSStatus AudioHub_RemoveDeviceClient(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/, const AudioServerPlugInClientInfo * /*inClientInfo*/) {
    return 0;
}

static OSStatus AudioHub_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_PerformDeviceConfigurationChange: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_PerformDeviceConfigurationChange: unknown device");
        theDevice->PerformConfigChange(inChangeAction, inChangeInfo);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_PerformDeviceConfigurationChange: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_PerformDeviceConfigurationChange: unknown device");
        theDevice->AbortConfigChange(inChangeAction, inChangeInfo);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

#pragma mark Property Operations

static Boolean AudioHub_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress) {
    Boolean theAnswer = false;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_HasProperty: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "AudioHub_HasProperty: no address");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_HasProperty: unknown object");
        theAnswer = theObject->HasProperty(inObjectID, inClientProcessID, *inAddress);
    }
    catch (const CAException &inException) {
        theAnswer = false;
    }
    catch (...) {
        theAnswer = false;
    }
    return theAnswer;
}

static OSStatus AudioHub_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_IsPropertySettable: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "AudioHub_IsPropertySettable: no address");
        ThrowIfNULL(outIsSettable, CAException(kAudioHardwareIllegalOperationError), "AudioHub_IsPropertySettable: no place to put the return value");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_IsPropertySettable: unknown object");
        if (theObject->HasProperty(inObjectID, inClientProcessID, *inAddress)) {
            *outIsSettable = theObject->IsPropertySettable(inObjectID, inClientProcessID, *inAddress);
        }
        else {
            theAnswer = kAudioHardwareUnknownPropertyError;
        }
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *outDataSize) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_GetPropertyDataSize: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetPropertyDataSize: no address");
        ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetPropertyDataSize: no place to put the return value");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_GetPropertyDataSize: unknown object");
        if (theObject->HasProperty(inObjectID, inClientProcessID, *inAddress)) {
            *outDataSize = theObject->GetPropertyDataSize(inObjectID, inClientProcessID, *inAddress, inQualifierDataSize, inQualifierData);
        }
        else {
            theAnswer = kAudioHardwareUnknownPropertyError;
        }
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 *outDataSize, void *outData) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_GetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetPropertyData: no address");
        ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetPropertyData: no place to put the return value size");
        ThrowIfNULL(outData, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetPropertyData: no place to put the return value");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_GetPropertyData: unknown object");
        if (theObject->HasProperty(inObjectID, inClientProcessID, *inAddress)) {
            theObject->GetPropertyData(inObjectID, inClientProcessID, *inAddress, inQualifierDataSize, inQualifierData, inDataSize, *outDataSize, outData);
        }
        else {
            theAnswer = kAudioHardwareUnknownPropertyError;
        }
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_SetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "AudioHub_SetPropertyData: no address");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_SetPropertyData: unknown object");
        if (theObject->HasProperty(inObjectID, inClientProcessID, *inAddress)) {
            if (theObject->IsPropertySettable(inObjectID, inClientProcessID, *inAddress)) {
                theObject->SetPropertyData(inObjectID, inClientProcessID, *inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
            }
            else {
                theAnswer = kAudioHardwareUnsupportedOperationError;
            }
        }
        else {
            theAnswer = kAudioHardwareUnknownPropertyError;
        }
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

#pragma mark IO Operations

static OSStatus AudioHub_StartIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_StartIO: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_StartIO: unknown device");
        theDevice->StartIO();
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_StopIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_StopIO: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_StopIO: unknown device");
        theDevice->StopIO();
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }

    return theAnswer;
}

static OSStatus AudioHub_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, Float64 *outSampleTime, UInt64 *outHostTime, UInt64 *outSeed) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_GetZeroTimeStamp: bad driver reference");
        ThrowIfNULL(outSampleTime, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetZeroTimeStamp: no place to put the sample time");
        ThrowIfNULL(outHostTime, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetZeroTimeStamp: no place to put the host time");
        ThrowIfNULL(outSeed, CAException(kAudioHardwareIllegalOperationError), "AudioHub_GetZeroTimeStamp: no place to put the seed");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_GetZeroTimeStamp: unknown device");
        theDevice->GetZeroTimeStamp(*outSampleTime, *outHostTime, *outSeed);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_WillDoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, Boolean *outWillDo, Boolean *outWillDoInPlace) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_WillDoIOOperation: bad driver reference");
        ThrowIfNULL(outWillDo, CAException(kAudioHardwareIllegalOperationError), "AudioHub_WillDoIOOperation: no place to put the will-do return value");
        ThrowIfNULL(outWillDoInPlace, CAException(kAudioHardwareIllegalOperationError), "AudioHub_WillDoIOOperation: no place to put the in-place return value");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_WillDoIOOperation: unknown device");
        bool willDo = false;
        bool willDoInPlace = false;
        theDevice->WillDoIOOperation(inOperationID, willDo, willDoInPlace);
        *outWillDo = willDo;
        *outWillDoInPlace = willDoInPlace;
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_BeginIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_BeginIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "AudioHub_BeginIOOperation: no cycle info");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_BeginIOOperation: unknown device");
        theDevice->BeginIOOperation(inOperationID, inIOBufferFrameSize, *inIOCycleInfo);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_DoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, AudioObjectID inStreamObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo, void *ioMainBuffer, void *ioSecondaryBuffer) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_DoIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "AudioHub_DoIOOperation: no cycle info");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_DoIOOperation: unknown device");
        theDevice->DoIOOperation(inStreamObjectID, inOperationID, inIOBufferFrameSize, *inIOCycleInfo, ioMainBuffer, ioSecondaryBuffer);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus AudioHub_EndIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "AudioHub_EndIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "AudioHub_EndIOOperation: no cycle info");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "AudioHub_EndIOOperation: unknown device");
        theDevice->EndIOOperation(inOperationID, inIOBufferFrameSize, *inIOCycleInfo);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}
