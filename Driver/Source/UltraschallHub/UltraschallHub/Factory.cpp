//
// Created by Daniel Lindenfelser on 04/11/14.
// Copyright (c) 2014 ultraschall.fm. All rights reserved.
//

#include "Factory.h"

#include "PlugIn.h"
#include "Device.h"
#include "CACFObject.h"
#include "CAException.h"

static AudioServerPlugInDriverInterface *gAudioServerPlugInDriverInterfacePtr = &gAudioServerPlugInDriverInterface;
static AudioServerPlugInDriverRef gAudioServerPlugInDriverRef = &gAudioServerPlugInDriverInterfacePtr;
static UInt32 gAudioServerPlugInDriverRefCount = 1;

#pragma mark Factory

extern "C"
__unused void *UltHub_Create(CFAllocatorRef /*inAllocator*/, CFUUIDRef inRequestedTypeUUID) {
    //	This is the CFPlugIn factory function. Its job is to create the implementation for the given
    //	type provided that the type is supported. Because this driver is simple and all its
    //	initialization is handled via static iniitalization when the bundle is loaded, all that
    //	needs to be done is to return the AudioServerPlugInDriverRef that points to the driver's
    //	interface. A more complicated driver would create any base line objects it needs to satisfy
    //	the IUnknown methods that are used to discover that actual interface to talk to the driver.
    //	The majority of the driver's initilization should be handled in the Initialize() method of
    //	the driver's AudioServerPlugInDriverInterface.

    void *theAnswer = NULL;
    if (CFEqual(inRequestedTypeUUID, kAudioServerPlugInTypeUUID)) {
        theAnswer = gAudioServerPlugInDriverRef;
        UltHub_PlugIn::GetInstance();
    }
    return theAnswer;
}


#pragma mark Inheritence

static HRESULT UltHub_QueryInterface(void *inDriver, REFIID inUUID, LPVOID *outInterface) {
    //	This function is called by the HAL to get the interface to talk to the plug-in through.
    //	AudioServerPlugIns are required to support the IUnknown interface and the
    //	AudioServerPlugInDriverInterface. As it happens, all interfaces must also provide the
    //	IUnknown interface, so we can always just return the single interface we made with
    //	gAudioServerPlugInDriverInterfacePtr regardless of which one is asked for.

    //	declare the local variables
    HRESULT theAnswer = 0;

    try {
        //	validate the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_QueryInterface: bad driver reference");
        ThrowIfNULL(outInterface, CAException(kAudioHardwareIllegalOperationError), "UltHub_QueryInterface: no place to store the returned interface");

        //	make a CFUUIDRef from inUUID
        CACFUUID theRequestedUUID(CFUUIDCreateFromUUIDBytes(NULL, inUUID));
        ThrowIf(!theRequestedUUID.IsValid(), CAException(kAudioHardwareIllegalOperationError), "UltHub_QueryInterface: failed to create the CFUUIDRef");

        //	AudioServerPlugIns only support two interfaces, IUnknown (which has to be supported by all
        //	CFPlugIns and AudioServerPlugInDriverInterface (which is the actual interface the HAL will
        //	use).
        ThrowIf(!CFEqual(theRequestedUUID.GetCFObject(), IUnknownUUID) && !CFEqual(theRequestedUUID.GetCFObject(), kAudioServerPlugInDriverInterfaceUUID), CAException(E_NOINTERFACE), "UltHub_QueryInterface: requested interface is unsupported");
        ThrowIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, CAException(E_NOINTERFACE), "UltHub_QueryInterface: the ref count is maxxed out");

        //	do the work
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

static ULONG UltHub_AddRef(void *inDriver) {
    //	This call returns the resulting reference count after the increment.

    //	declare the local variables
    ULONG theAnswer = 0;

    //	check the arguments
    FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "UltHub_AddRef: bad driver reference");
    FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "UltHub_AddRef: out of references");

    //	increment the refcount
    ++gAudioServerPlugInDriverRefCount;
    theAnswer = gAudioServerPlugInDriverRefCount;

    Done:
    return theAnswer;
}

static ULONG UltHub_Release(void *inDriver) {
    //	This call returns the resulting reference count after the decrement.

    //	declare the local variables
    ULONG theAnswer = 0;

    //	check the arguments
    FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "UltHub_Release: bad driver reference");
    FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "UltHub_Release: out of references");

    //	decrement the refcount
    //	Note that we don't do anything special if the refcount goes to zero as the HAL
    //	will never fully release a plug-in it opens. We keep managing the refcount so that
    //	the API semantics are correct though.
    --gAudioServerPlugInDriverRefCount;
    theAnswer = gAudioServerPlugInDriverRefCount;

    Done:
    return theAnswer;
}

#pragma mark Basic Operations

static OSStatus UltHub_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost) {
    //	The job of this method is, as the name implies, to get the driver initialized. One specific
    //	thing that needs to be done is to store the AudioServerPlugInHostRef so that it can be used
    //	later. Note that when this call returns, the HAL will scan the various lists the driver
    //	maintains (such as the device list) to get the inital set of objects the driver is
    //	publishing. So, there is no need to notifiy the HAL about any objects created as part of the
    //	execution of this method.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_Initialize: bad driver reference");

        //	store the AudioServerPlugInHostRef
        UltHub_PlugIn::GetInstance().SetHost(inHost);
    }
    catch (const CAException &inException) {
        theAnswer = inException.GetError();
    }
    catch (...) {
        theAnswer = kAudioHardwareUnspecifiedError;
    }

    return theAnswer;
}

static OSStatus UltHub_CreateDevice(AudioServerPlugInDriverRef /*inDriver*/, CFDictionaryRef /*inDescription*/, const AudioServerPlugInClientInfo * /*inClientInfo*/, AudioObjectID * /*outDeviceObjectID*/) {
    //	This method is used to tell a driver that implements the Transport Manager semantics to
    //	create an AudioEndpointDevice from a set of AudioEndpoints. Since this driver is not a
    //	Transport Manager, we just return kAudioHardwareUnsupportedOperationError.

    return kAudioHardwareUnsupportedOperationError;
}

static OSStatus UltHub_DestroyDevice(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/) {
    //	This method is used to tell a driver that implements the Transport Manager semantics to
    //	destroy an AudioEndpointDevice. Since this driver is not a Transport Manager, we just check
    //	the arguments and return kAudioHardwareUnsupportedOperationError.

    return kAudioHardwareUnsupportedOperationError;
}

static OSStatus UltHub_AddDeviceClient(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/, const AudioServerPlugInClientInfo * /*inClientInfo*/) {
    //	This method is used to inform the driver about a new client that is using the given device.
    //	This allows the device to act differently depending on who the client is. This driver does
    //	not need to track the clients using the device, so we just return successfully.

    return 0;
}

static OSStatus UltHub_RemoveDeviceClient(AudioServerPlugInDriverRef /*inDriver*/, AudioObjectID /*inDeviceObjectID*/, const AudioServerPlugInClientInfo * /*inClientInfo*/) {
    //	This method is used to inform the driver about a client that is no longer using the given
    //	device. This driver does not track clients, so we just return successfully.

    return 0;
}

static OSStatus UltHub_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
    //	This method is called to tell the device that it can perform the configuation change that it
    //	had requested via a call to the host method, RequestDeviceConfigurationChange(). The
    //	arguments, inChangeAction and inChangeInfo are the same as what was passed to
    //	RequestDeviceConfigurationChange().
    //
    //	The HAL guarantees that IO will be stopped while this method is in progress. The HAL will
    //	also handle figuring out exactly what changed for the non-control related properties. This
    //	means that the only notifications that would need to be sent here would be for either
    //	custom properties the HAL doesn't know about or for controls.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_PerformDeviceConfigurationChange: bad driver reference");

        //	get the device object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_PerformDeviceConfigurationChange: unknown device");

        //	tell it to do the work
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

static OSStatus UltHub_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void *inChangeInfo) {
    //	This method is called to tell the driver that a request for a config change has been denied.
    //	This provides the driver an opportunity to clean up any state associated with the request.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_PerformDeviceConfigurationChange: bad driver reference");

        //	get the device object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_PerformDeviceConfigurationChange: unknown device");

        //	tell it to do the work
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

static Boolean UltHub_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress) {
    //	This method returns whether or not the given object has the given property.

    //	declare the local variables
    Boolean theAnswer = false;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_HasProperty: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "UltHub_HasProperty: no address");

        //	get the object
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_HasProperty: unknown object");

        //	tell it to do the work
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

static OSStatus UltHub_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable) {
    //	This method returns whether or not the given property on the object can have its value
    //	changed.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_IsPropertySettable: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "UltHub_IsPropertySettable: no address");
        ThrowIfNULL(outIsSettable, CAException(kAudioHardwareIllegalOperationError), "UltHub_IsPropertySettable: no place to put the return value");

        //	get the object
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_IsPropertySettable: unknown object");

        //	tell it to do the work
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

static OSStatus UltHub_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *outDataSize) {
    //	This method returns the byte size of the property's data.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_GetPropertyDataSize: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetPropertyDataSize: no address");
        ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetPropertyDataSize: no place to put the return value");

        //	get the object
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_GetPropertyDataSize: unknown object");

        //	tell it to do the work
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

static OSStatus UltHub_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 *outDataSize, void *outData) {
    //	This method fetches the data for a given property

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_GetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetPropertyData: no address");
        ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetPropertyData: no place to put the return value size");
        ThrowIfNULL(outData, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetPropertyData: no place to put the return value");

        //	get the object
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_GetPropertyData: unknown object");

        //	tell it to do the work
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

static OSStatus UltHub_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    //	This method changes the value of the given property

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_SetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "UltHub_SetPropertyData: no address");

        //	get the object
        CAObjectReleaser<CAObject> theObject(CAObjectMap::CopyObjectByObjectID(inObjectID));
        ThrowIf(!theObject.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_SetPropertyData: unknown object");

        //	tell it to do the work
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

static OSStatus UltHub_StartIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/) {
    //	This call tells the device that IO is starting for the given client. When this routine
    //	returns, the device's clock is running and it is ready to have data read/written. It is
    //	important to note that multiple clients can have IO running on the device at the same time.
    //	So, work only needs to be done when the first client starts. All subsequent starts simply
    //	increment the counter.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_StartIO: bad driver reference");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_StartIO: unknown device");

        //	tell it to do the work
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

static OSStatus UltHub_StopIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/) {
    //	This call tells the device that the client has stopped IO. The driver can stop the hardware
    //	once all clients have stopped.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_StopIO: bad driver reference");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_StopIO: unknown device");

        //	tell it to do the work
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

static OSStatus UltHub_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, Float64 *outSampleTime, UInt64 *outHostTime, UInt64 *outSeed) {
    //	This method returns the current zero time stamp for the device. The HAL models the timing of
    //	a device as a series of time stamps that relate the sample time to a host time. The zero
    //	time stamps are spaced such that the sample times are the value of
    //	kAudioDevicePropertyZeroTimeStampPeriod apart. This is often modeled using a ring buffer
    //	where the zero time stamp is updated when wrapping around the ring buffer.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_GetZeroTimeStamp: bad driver reference");
        ThrowIfNULL(outSampleTime, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetZeroTimeStamp: no place to put the sample time");
        ThrowIfNULL(outHostTime, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetZeroTimeStamp: no place to put the host time");
        ThrowIfNULL(outSeed, CAException(kAudioHardwareIllegalOperationError), "UltHub_GetZeroTimeStamp: no place to put the seed");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_GetZeroTimeStamp: unknown device");

        //	tell it to do the work
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

static OSStatus UltHub_WillDoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, Boolean *outWillDo, Boolean *outWillDoInPlace) {
    //	This method returns whether or not the device will do a given IO operation.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_WillDoIOOperation: bad driver reference");
        ThrowIfNULL(outWillDo, CAException(kAudioHardwareIllegalOperationError), "UltHub_WillDoIOOperation: no place to put the will-do return value");
        ThrowIfNULL(outWillDoInPlace, CAException(kAudioHardwareIllegalOperationError), "UltHub_WillDoIOOperation: no place to put the in-place return value");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_WillDoIOOperation: unknown device");

        //	tell it to do the work
        bool willDo = false;
        bool willDoInPlace = false;
        theDevice->WillDoIOOperation(inOperationID, willDo, willDoInPlace);

        //	set the return values
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

static OSStatus UltHub_BeginIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo) {
    //	This is called at the beginning of an IO operation.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_BeginIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "UltHub_BeginIOOperation: no cycle info");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_BeginIOOperation: unknown device");

        //	tell it to do the work
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

static OSStatus UltHub_DoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, AudioObjectID inStreamObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo, void *ioMainBuffer, void *ioSecondaryBuffer) {
    //	This is called to actuall perform a given operation. For this device, all we need to do is
    //	clear the buffer for the ReadInput operation.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_EndIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "UltHub_EndIOOperation: no cycle info");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_EndIOOperation: unknown device");

        //	tell it to do the work
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

static OSStatus UltHub_EndIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 /*inClientID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo *inIOCycleInfo) {
    //	This is called at the end of an IO operation.

    //	declare the local variables
    OSStatus theAnswer = 0;

    try {
        //	check the arguments
        ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "UltHub_EndIOOperation: bad driver reference");
        ThrowIfNULL(inIOCycleInfo, CAException(kAudioHardwareIllegalOperationError), "UltHub_EndIOOperation: no cycle info");

        //	get the object
        CAObjectReleaser<UltHub_Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(inDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "UltHub_EndIOOperation: unknown device");

        //	tell it to do the work
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
