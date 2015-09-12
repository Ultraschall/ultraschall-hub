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

static AudioServerPlugInDriverInterface *gAudioServerPlugInDriverInterfacePtr = &gAudioServerPlugInDriverInterface;
static AudioServerPlugInDriverRef gAudioServerPlugInDriverRef = &gAudioServerPlugInDriverInterfacePtr;
static UInt32 gAudioServerPlugInDriverRefCount = 1;

#pragma mark Factory

extern "C" void *Hub_Create(CFAllocatorRef /*inAllocator*/, CFUUIDRef inRequestedTypeUUID) {
    void *theAnswer = NULL;
    if (CFEqual(inRequestedTypeUUID, kAudioServerPlugInTypeUUID)) {
        theAnswer = gAudioServerPlugInDriverRef;
        PlugIn::GetInstance();
    }
    return theAnswer;
}

#pragma mark Inheritence

static HRESULT Hub_QueryInterface(void *inDriver, REFIID inUUID, LPVOID *outInterface) {
    HRESULT theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_QueryInterface: bad driver reference");
        ThrowIfNULL(outInterface, CAException(kAudioHardwareIllegalOperationError), "Hub_QueryInterface: no place to store the returned interface");

        CACFUUID theRequestedUUID(CFUUIDCreateFromUUIDBytes(NULL, inUUID));
        ThrowIf(!theRequestedUUID.IsValid(), CAException(kAudioHardwareIllegalOperationError), "Hub_QueryInterface: failed to create the CFUUIDRef");
        ThrowIf(!CFEqual(theRequestedUUID.GetCFObject(), IUnknownUUID) && !CFEqual(theRequestedUUID.GetCFObject(), kAudioServerPlugInDriverInterfaceUUID), CAException(E_NOINTERFACE), "Hub_QueryInterface: requested interface is unsupported");
        ThrowIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, CAException(E_NOINTERFACE), "Hub_QueryInterface: the ref count is maxxed out");
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

static ULONG Hub_AddRef(void *inDriver) {
    ULONG theAnswer = 0;
    FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "Hub_AddRef: bad driver reference");
    FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "Hub_AddRef: out of references");
    ++gAudioServerPlugInDriverRefCount;
    theAnswer = gAudioServerPlugInDriverRefCount;
    Done:
    return theAnswer;
}

static ULONG Hub_Release(void *inDriver) {
    ULONG theAnswer = 0;
    FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "Hub_Release: bad driver reference");
    FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "Hub_Release: out of references");
    --gAudioServerPlugInDriverRefCount;
    theAnswer = gAudioServerPlugInDriverRefCount;
    Done:
    return theAnswer;
}

#pragma mark Basic Operations

static OSStatus Hub_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_Initialize: bad driver reference");
        PlugIn::GetInstance().SetHost(inHost);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }
    return theAnswer;
}

static OSStatus Hub_CreateDevice(AudioServerPlugInDriverRef /*inDriver*/, CFDictionaryRef /*inDescription*/, const AudioServerPlugInClientInfo * /*inClientInfo*/, AudioObjectID * /*outDeviceObjectID*/) {
    return kAudioHardwareUnsupportedOperationError;
}

static OSStatus Hub_DestroyDevice(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/) {
    return kAudioHardwareUnsupportedOperationError;
}

static OSStatus Hub_AddDeviceClient(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/, const AudioServerPlugInClientInfo * /*inClientInfo*/) {
    return 0;
}

static OSStatus Hub_RemoveDeviceClient(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/, const AudioServerPlugInClientInfo * /*inClientInfo*/) {
    return 0;
}

static OSStatus Hub_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_PerformDeviceConfigurationChange: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_PerformDeviceConfigurationChange: unknown device");
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

static OSStatus Hub_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_PerformDeviceConfigurationChange: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_PerformDeviceConfigurationChange: unknown device");
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

static Boolean Hub_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress) {
    Boolean theAnswer = false;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_HasProperty: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "Hub_HasProperty: no address");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_HasProperty: unknown object");
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

static OSStatus Hub_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_IsPropertySettable: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "Hub_IsPropertySettable: no address");
        ThrowIfNULL(outIsSettable, CAException(kAudioHardwareIllegalOperationError), "Hub_IsPropertySettable: no place to put the return value");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_IsPropertySettable: unknown object");
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

static OSStatus Hub_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *outDataSize) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_GetPropertyDataSize: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "Hub_GetPropertyDataSize: no address");
        ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "Hub_GetPropertyDataSize: no place to put the return value");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_GetPropertyDataSize: unknown object");
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

static OSStatus Hub_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 *outDataSize, void *outData) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_GetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "Hub_GetPropertyData: no address");
        ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "Hub_GetPropertyData: no place to put the return value size");
        ThrowIfNULL(outData, CAException(kAudioHardwareIllegalOperationError), "Hub_GetPropertyData: no place to put the return value");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_GetPropertyData: unknown object");
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

static OSStatus Hub_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_SetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "Hub_SetPropertyData: no address");
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_SetPropertyData: unknown object");
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

static OSStatus Hub_StartIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_StartIO: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_StartIO: unknown device");
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

static OSStatus Hub_StopIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_StopIO: bad driver reference");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_StopIO: unknown device");
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

static OSStatus Hub_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, Float64 *outSampleTime, UInt64 *outHostTime, UInt64 *outSeed) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_GetZeroTimeStamp: bad driver reference");
        ThrowIfNULL(outSampleTime, CAException(kAudioHardwareIllegalOperationError), "Hub_GetZeroTimeStamp: no place to put the sample time");
        ThrowIfNULL(outHostTime, CAException(kAudioHardwareIllegalOperationError), "Hub_GetZeroTimeStamp: no place to put the host time");
        ThrowIfNULL(outSeed, CAException(kAudioHardwareIllegalOperationError), "Hub_GetZeroTimeStamp: no place to put the seed");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_GetZeroTimeStamp: unknown device");
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

static OSStatus Hub_WillDoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, Boolean *outWillDo, Boolean *outWillDoInPlace) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_WillDoIOOperation: bad driver reference");
        ThrowIfNULL(outWillDo, CAException(kAudioHardwareIllegalOperationError), "Hub_WillDoIOOperation: no place to put the will-do return value");
        ThrowIfNULL(outWillDoInPlace, CAException(kAudioHardwareIllegalOperationError), "Hub_WillDoIOOperation: no place to put the in-place return value");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_WillDoIOOperation: unknown device");
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

static OSStatus Hub_BeginIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_BeginIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "Hub_BeginIOOperation: no cycle info");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_BeginIOOperation: unknown device");
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

static OSStatus Hub_DoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, AudioObjectID inStreamObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo, void *ioMainBuffer, void *ioSecondaryBuffer) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_DoIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "Hub_DoIOOperation: no cycle info");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_DoIOOperation: unknown device");
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

static OSStatus Hub_EndIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo) {
    OSStatus theAnswer = 0;
    try {
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "Hub_EndIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "Hub_EndIOOperation: no cycle info");
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "Hub_EndIOOperation: unknown device");
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
