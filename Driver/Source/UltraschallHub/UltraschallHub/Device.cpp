#include <mach/mach_time.h>
#include <sys/cdefs.h> //
//  Driver.cpp
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 02/11/14.
//  Copyright (c) 2014 ultraschall.fm. All rights reserved.
//

#include "Device.h"

#include "PlugIn.h"

#include "CADispatchQueue.h"
#include "CAException.h"

#pragma mark Construction/Destruction

UltHub_Device::UltHub_Device(AudioObjectID inObjectID, SInt16 numChannels)
    : CAObject(inObjectID, kAudioDeviceClassID, kAudioObjectClassID, kAudioObjectPlugInObject)
    , mStateMutex(new CAMutex("Ultraschall State"))
    , mIOMutex(new CAMutex("Ultraschall IO"))
    , mStartCount(0)
    , mBufferSize(8192)
    , mDeviceUID("UltraschallHub")
    , mInputStreamObjectID(CAObjectMap::GetNextObjectID())
    , mInputStreamIsActive(true)
    , mOutputStreamObjectID(CAObjectMap::GetNextObjectID())
    , mOutputStreamIsActive(true)
    , mInputMasterVolumeControlObjectID(CAObjectMap::GetNextObjectID())
    , mInputMasterVolumeControlRawValueShadow(kUltraschallHub_Control_MinRawVolumeValue)
    , mOutputMasterVolumeControlObjectID(CAObjectMap::GetNextObjectID())
    , mOutputMasterVolumeControlRawValueShadow(kUltraschallHub_Control_MinRawVolumeValue)
    , mVolumeCurve()
    , mNumChannels(numChannels)
    , mTimeline(0)
{
    mStreamDescription.mFormatID = kAudioFormatLinearPCM;
    mStreamDescription.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
    mStreamDescription.mFramesPerPacket = 1;
    mStreamDescription.mBytesPerPacket = mNumChannels * sizeof(Float32);
    mStreamDescription.mBytesPerFrame = mNumChannels * sizeof(Float32);
    mStreamDescription.mChannelsPerFrame = mNumChannels;
    mStreamDescription.mBitsPerChannel = sizeof(Float32) * 8;
    mStreamDescription.mSampleRate = 44100;

    //	Setup the volume curve with the one range
    mVolumeCurve.AddRange(kUltraschallHub_Control_MinRawVolumeValue, kUltraschallHub_Control_MaxRawVolumeValue, kUltraschallHub_Control_MinDBVolumeValue, kUltraschallHub_Control_MaxDbVolumeValue);
}

void UltHub_Device::Activate()
{
    //	map the subobject IDs to this object
    CAObjectMap::MapObject(mInputStreamObjectID, this);
    CAObjectMap::MapObject(mOutputStreamObjectID, this);
    CAObjectMap::MapObject(mInputMasterVolumeControlObjectID, this);
    CAObjectMap::MapObject(mOutputMasterVolumeControlObjectID, this);

    //	call the super-class, which just marks the object as active
    CAObject::Activate();
}

void UltHub_Device::Deactivate()
{
    //	When this method is called, the object is basically dead, but we still need to be thread
    //	safe. In this case, we also need to be safe vs. any IO threads, so we need to take both
    //	locks.
    CAMutex::Locker theStateLocker(mStateMutex);
    CAMutex::Locker theIOLocker(mIOMutex);

    //	mark the object inactive by calling the super-class
    CAObject::Deactivate();

    //	unmap the subobject IDs
    CAObjectMap::UnmapObject(mInputStreamObjectID, this);
    CAObjectMap::UnmapObject(mOutputStreamObjectID, this);
    CAObjectMap::UnmapObject(mInputMasterVolumeControlObjectID, this);
    CAObjectMap::UnmapObject(mOutputMasterVolumeControlObjectID, this);
}

UltHub_Device::~UltHub_Device()
{
    delete mStateMutex;
    delete mIOMutex;
    mStateMutex = nullptr;
    mIOMutex = nullptr;
}

#pragma mark Property Operations

bool UltHub_Device::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	This object implements several API-level objects. So the first thing to do is to figure out
    //    	which object this request is really for. Note that mSubObjectID is an invariant as this
    //	driver's structure does not change dynamically. It will always have the parts it has.
    bool theAnswer;
    if (inObjectID == mObjectID) {
        theAnswer = Device_HasProperty(inObjectID, inClientPID, inAddress);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        theAnswer = Stream_HasProperty(inObjectID, inClientPID, inAddress);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        theAnswer = Control_HasProperty(inObjectID, inClientPID, inAddress);
    }
    else {
        Throw(CAException(kAudioHardwareBadObjectError));
    }
    return theAnswer;
}

bool UltHub_Device::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer;
    if (inObjectID == mObjectID) {
        theAnswer = Device_IsPropertySettable(inObjectID, inClientPID, inAddress);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        theAnswer = Stream_IsPropertySettable(inObjectID, inClientPID, inAddress);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        theAnswer = Control_IsPropertySettable(inObjectID, inClientPID, inAddress);
    }
    else {
        Throw(CAException(kAudioHardwareBadObjectError));
    }
    return theAnswer;
}

UInt32 UltHub_Device::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    UInt32 theAnswer = 0;
    if (inObjectID == mObjectID) {
        theAnswer = Device_GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        theAnswer = Stream_GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        theAnswer = Control_GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
    }
    else {
        Throw(CAException(kAudioHardwareBadObjectError));
    }
    return theAnswer;
}

void UltHub_Device::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
    if (inObjectID == mObjectID) {
        Device_GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        Stream_GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        Control_GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
    }
    else {
        Throw(CAException(kAudioHardwareBadObjectError));
    }
}

void UltHub_Device::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    if (inObjectID == mObjectID) {
        Device_SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        Stream_SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        Control_SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
    }
    else {
        Throw(CAException(kAudioHardwareBadObjectError));
    }
}

#pragma mark Device Property Operations

bool UltHub_Device::Device_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Device_GetPropertyData() method.

    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyName:
    case kAudioObjectPropertyManufacturer:
    case kAudioDevicePropertyDeviceUID:
    case kAudioDevicePropertyModelUID:
    case kAudioDevicePropertyTransportType:
    case kAudioDevicePropertyRelatedDevices:
    case kAudioDevicePropertyClockDomain:
    case kAudioDevicePropertyDeviceIsAlive:
    case kAudioDevicePropertyDeviceIsRunning:
    case kAudioObjectPropertyControlList:
    case kAudioDevicePropertyNominalSampleRate:
    case kAudioDevicePropertyAvailableNominalSampleRates:
    case kAudioDevicePropertyIsHidden:
    case kAudioDevicePropertyZeroTimeStampPeriod:
    case kAudioDevicePropertyStreams:
        theAnswer = true;
        break;

    case kAudioDevicePropertyLatency:
    case kAudioDevicePropertySafetyOffset:
    case kAudioDevicePropertyPreferredChannelsForStereo:
    case kAudioDevicePropertyPreferredChannelLayout:
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
        theAnswer = (inAddress.mScope == kAudioObjectPropertyScopeInput) || (inAddress.mScope == kAudioObjectPropertyScopeOutput);
        break;

    default:
        theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
        break;
    };
    return theAnswer;
}

bool UltHub_Device::Device_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Device_GetPropertyData() method.

    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyName:
    case kAudioObjectPropertyManufacturer:
    case kAudioDevicePropertyDeviceUID:
    case kAudioDevicePropertyModelUID:
    case kAudioDevicePropertyTransportType:
    case kAudioDevicePropertyRelatedDevices:
    case kAudioDevicePropertyClockDomain:
    case kAudioDevicePropertyDeviceIsAlive:
    case kAudioDevicePropertyDeviceIsRunning:
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
    case kAudioDevicePropertyLatency:
    case kAudioDevicePropertyStreams:
    case kAudioObjectPropertyControlList:
    case kAudioDevicePropertySafetyOffset:
    case kAudioDevicePropertyAvailableNominalSampleRates:
    case kAudioDevicePropertyIsHidden:
    case kAudioDevicePropertyPreferredChannelsForStereo:
    case kAudioDevicePropertyPreferredChannelLayout:
    case kAudioDevicePropertyZeroTimeStampPeriod:
        theAnswer = false;
        break;

    case kAudioDevicePropertyNominalSampleRate:
        theAnswer = true;
        break;

    default:
        theAnswer = CAObject::IsPropertySettable(inObjectID, inClientPID, inAddress);
        break;
    };
    return theAnswer;
}

UInt32 UltHub_Device::Device_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Device_GetPropertyData() method.

    UInt32 theAnswer = 0;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyName:
        theAnswer = sizeof(CFStringRef);
        break;

    case kAudioObjectPropertyManufacturer:
        theAnswer = sizeof(CFStringRef);
        break;

    case kAudioObjectPropertyOwnedObjects:
        switch (inAddress.mScope) {
        case kAudioObjectPropertyScopeGlobal:
            theAnswer = kNumberOfSubObjects * sizeof(AudioObjectID);
            break;

        case kAudioObjectPropertyScopeInput:
            theAnswer = kNumberOfInputSubObjects * sizeof(AudioObjectID);
            break;

        case kAudioObjectPropertyScopeOutput:
            theAnswer = kNumberOfOutputSubObjects * sizeof(AudioObjectID);
            break;
        case kAudioObjectPropertyScopePlayThrough:
            break;
        case kAudioObjectPropertyScopeWildcard:
            break;
        };
        break;

    case kAudioDevicePropertyDeviceUID:
        theAnswer = sizeof(CFStringRef);
        break;

    case kAudioDevicePropertyModelUID:
        theAnswer = sizeof(CFStringRef);
        break;

    case kAudioDevicePropertyTransportType:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyRelatedDevices:
        theAnswer = sizeof(AudioObjectID);
        break;

    case kAudioDevicePropertyClockDomain:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyDeviceIsAlive:
        theAnswer = sizeof(AudioClassID);
        break;

    case kAudioDevicePropertyDeviceIsRunning:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyLatency:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyStreams:
        switch (inAddress.mScope) {
        case kAudioObjectPropertyScopeGlobal:
            theAnswer = kNumberOfStreams * sizeof(AudioObjectID);
            break;

        case kAudioObjectPropertyScopeInput:
            theAnswer = kNumberOfInputStreams * sizeof(AudioObjectID);
            break;

        case kAudioObjectPropertyScopeOutput:
            theAnswer = kNumberOfOutputStreams * sizeof(AudioObjectID);
            break;
        case kAudioObjectPropertyScopePlayThrough:
            break;
        case kAudioObjectPropertyScopeWildcard:
            break;
        };
        break;

    case kAudioObjectPropertyControlList:
        theAnswer = kNumberOfControls * sizeof(AudioObjectID);
        break;

    case kAudioDevicePropertySafetyOffset:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyNominalSampleRate:
        theAnswer = sizeof(Float64);
        break;

    case kAudioDevicePropertyAvailableNominalSampleRates:
        theAnswer = 6 * sizeof(AudioValueRange);
        break;

    case kAudioDevicePropertyIsHidden:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioDevicePropertyPreferredChannelsForStereo:
        theAnswer = 2 * sizeof(UInt32);
        break;

    case kAudioDevicePropertyPreferredChannelLayout:
        theAnswer = offsetof(AudioChannelLayout, mChannelDescriptions) + (2 * sizeof(AudioChannelDescription));
        break;

    case kAudioDevicePropertyZeroTimeStampPeriod:
        theAnswer = sizeof(UInt32);
        break;

    default:
        theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
        break;
    };
    return theAnswer;
}

void UltHub_Device::Device_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required.
    //	Also, since most of the data that will get returned is static, there are few instances where
    //	it is necessary to lock the state mutex.

    UInt32 theNumberItemsToFetch;
    UInt32 theItemIndex;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyName:
        //	This is the human readable name of the device. Note that in this case we return a
        //	value that is a key into the localizable strings in this bundle. This allows us to
        //	return a localized name for the device.
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer for the device");
        *reinterpret_cast<CFStringRef*>(outData) = mDeviceName.CopyCFString();
        outDataSize = sizeof(CFStringRef);
        break;

    case kAudioObjectPropertyManufacturer:
        //	This is the human readable name of the maker of the plug-in. Note that in this case
        //	we return a value that is a key into the localizable strings in this bundle. This
        //	allows us to return a localized name for the manufacturer.
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer for the device");
        *reinterpret_cast<CFStringRef*>(outData) = CFSTR("ultraschall.fm");
        outDataSize = sizeof(CFStringRef);
        break;

    case kAudioObjectPropertyOwnedObjects:
        //	Calculate the number of items that have been requested. Note that this
        //	number is allowed to be smaller than the actual size of the list. In such
        //	case, only that number of items will be returned
        theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioObjectID));

        //	The device owns its streams and controls. Note that what is returned here
        //	depends on the scope requested.
        switch (inAddress.mScope) {
        case kAudioObjectPropertyScopeGlobal:
            //	global scope means return all objects
            if (theNumberItemsToFetch > kNumberOfSubObjects) {
                theNumberItemsToFetch = kNumberOfSubObjects;
            }

            //	fill out the list with as many objects as requested, which is everything
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID*>(outData)[0] = mInputStreamObjectID;
            }
            if (theNumberItemsToFetch > 1) {
                reinterpret_cast<AudioObjectID*>(outData)[1] = mOutputStreamObjectID;
            }
            if (theNumberItemsToFetch > 2) {
                reinterpret_cast<AudioObjectID*>(outData)[2] = mInputMasterVolumeControlObjectID;
            }
            if (theNumberItemsToFetch > 3) {
                reinterpret_cast<AudioObjectID*>(outData)[3] = mOutputMasterVolumeControlObjectID;
            }
            break;

        case kAudioObjectPropertyScopeInput:
            //	input scope means just the objects on the input side
            if (theNumberItemsToFetch > kNumberOfInputSubObjects) {
                theNumberItemsToFetch = kNumberOfInputSubObjects;
            }

            //	fill out the list with the right objects
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID*>(outData)[0] = mInputStreamObjectID;
            }
            if (theNumberItemsToFetch > 1) {
                reinterpret_cast<AudioObjectID*>(outData)[1] = mInputMasterVolumeControlObjectID;
            }
            break;

        case kAudioObjectPropertyScopeOutput:
            //	output scope means just the objects on the output side
            if (theNumberItemsToFetch > kNumberOfOutputSubObjects) {
                theNumberItemsToFetch = kNumberOfOutputSubObjects;
            }

            //	fill out the list with the right objects
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID*>(outData)[0] = mOutputStreamObjectID;
            }
            if (theNumberItemsToFetch > 1) {
                reinterpret_cast<AudioObjectID*>(outData)[1] = mOutputMasterVolumeControlObjectID;
            }
            break;
        case kAudioObjectPropertyScopePlayThrough:
            break;
        case kAudioObjectPropertyScopeWildcard:
            break;
        };

        //	report how much we wrote
        outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioObjectID));
        break;

    case kAudioDevicePropertyDeviceUID:
        //	This is a CFString that is a persistent token that can identify the same
        //	audio device across boot sessions. Note that two instances of the same
        //	device must have different values for this property.
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceUID for the device");
        *reinterpret_cast<CFStringRef*>(outData) = mDeviceUID.CopyCFString();
        outDataSize = sizeof(CFStringRef);
        break;

    case kAudioDevicePropertyModelUID:
        //	This is a CFString that is a persistent token that can identify audio
        //	devices that are the same kind of device. Note that two instances of the
        //	save device must have the same value for this property.
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyModelUID for the device");
        *reinterpret_cast<CFStringRef*>(outData) = CFSTR("UltraschallHub");
        outDataSize = sizeof(CFStringRef);
        break;

    case kAudioDevicePropertyTransportType:
        //	This value represents how the device is attached to the system. This can be
        //	any 32 bit integer, but common values for this property are defined in
        //	<CoreAudio/AudioHardwareBase.h>
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyTransportType for the device");
        *reinterpret_cast<UInt32*>(outData) = kAudioDeviceTransportTypeVirtual;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyRelatedDevices:
        //	The related devices property identifies device objects that are very closely
        //	related. Generally, this is for relating devices that are packaged together
        //	in the hardware such as when the input side and the output side of a piece
        //	of hardware can be clocked separately and therefore need to be represented
        //	as separate AudioDevice objects. In such case, both devices would report
        //	that they are related to each other. Note that at minimum, a device is
        //	related to itself, so this list will always be at least one item long.

        //	Calculate the number of items that have been requested. Note that this
        //	number is allowed to be smaller than the actual size of the list. In such
        //	case, only that number of items will be returned
        theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioObjectID));

        //	we only have the one device...
        if (theNumberItemsToFetch > 1) {
            theNumberItemsToFetch = 1;
        }

        //	Write the devices' object IDs into the return value
        if (theNumberItemsToFetch > 0) {
            reinterpret_cast<AudioObjectID*>(outData)[0] = GetObjectID();
        }

        //	report how much we wrote
        outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioObjectID));
        break;

    case kAudioDevicePropertyClockDomain:
        //	This property allows the device to declare what other devices it is
        //	synchronized with in hardware. The way it works is that if two devices have
        //	the same value for this property and the value is not zero, then the two
        //	devices are synchronized in hardware. Note that a device that either can't
        //	be synchronized with others or doesn't know should return 0 for this
        //	property.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyClockDomain for the device");
        *reinterpret_cast<UInt32*>(outData) = 0;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyDeviceIsAlive:
        //	This property returns whether or not the device is alive. Note that it is
        //	note uncommon for a device to be dead but still momentarily availble in the
        //	device list. In the case of this device, it will always be alive.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceIsAlive for the device");
        *reinterpret_cast<UInt32*>(outData) = 1;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyDeviceIsRunning:
        //	This property returns whether or not IO is running for the device.
        {
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceIsRunning for the device");

            //	The IsRunning state is protected by the state lock
            CAMutex::Locker theStateLocker(mStateMutex);

            //	return the state and how much data we are touching
            *reinterpret_cast<UInt32*>(outData) = (UInt32)(mStartCount > 0);
            outDataSize = sizeof(UInt32);
        }
        break;

    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
        //	This property returns whether or not the device wants to be able to be the
        //	default device for content. This is the device that iTunes and QuickTime
        //	will use to play their content on and FaceTime will use as it's microhphone.
        //	Nearly all devices should allow for this.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceCanBeDefaultDevice for the device");
        *reinterpret_cast<UInt32*>(outData) = 1;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
        //	This property returns whether or not the device wants to be the system
        //	default device. This is the device that is used to play interface sounds and
        //	other incidental or UI-related sounds on. Most devices should allow this
        //	although devices with lots of latency may not want to.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceCanBeDefaultSystemDevice for the device");
        *reinterpret_cast<UInt32*>(outData) = 1;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyLatency:
        //	This property returns the presentation latency of the device. For this,
        //	device, the value is 0 due to the fact that it always vends silence.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyLatency for the device");
        *reinterpret_cast<UInt32*>(outData) = 0;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyStreams:
        //	Calculate the number of items that have been requested. Note that this
        //	number is allowed to be smaller than the actual size of the list. In such
        //	case, only that number of items will be returned
        theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioObjectID));

        //	Note that what is returned here depends on the scope requested.
        switch (inAddress.mScope) {
        case kAudioObjectPropertyScopeGlobal:
            //	global scope means return all streams
            if (theNumberItemsToFetch > kNumberOfStreams) {
                theNumberItemsToFetch = kNumberOfStreams;
            }

            //	fill out the list with as many objects as requested
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID*>(outData)[0] = mInputStreamObjectID;
            }
            if (theNumberItemsToFetch > 1) {
                reinterpret_cast<AudioObjectID*>(outData)[1] = mOutputStreamObjectID;
            }
            break;

        case kAudioObjectPropertyScopeInput:
            //	input scope means just the objects on the input side
            if (theNumberItemsToFetch > kNumberOfInputStreams) {
                theNumberItemsToFetch = kNumberOfInputStreams;
            }

            //	fill out the list with as many objects as requested
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID*>(outData)[0] = mInputStreamObjectID;
            }
            break;

        case kAudioObjectPropertyScopeOutput:
            //	output scope means just the objects on the output side
            if (theNumberItemsToFetch > kNumberOfOutputStreams) {
                theNumberItemsToFetch = kNumberOfOutputStreams;
            }

            //	fill out the list with as many objects as requested
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID*>(outData)[0] = mOutputStreamObjectID;
            }
            break;
        case kAudioObjectPropertyScopePlayThrough:
            break;
        case kAudioObjectPropertyScopeWildcard:
            break;
        };

        //	report how much we wrote
        outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioObjectID));
        break;

    case kAudioObjectPropertyControlList:
        //	Calculate the number of items that have been requested. Note that this
        //	number is allowed to be smaller than the actual size of the list. In such
        //	case, only that number of items will be returned
        theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioObjectID));
        if (theNumberItemsToFetch > kNumberOfControls) {
            theNumberItemsToFetch = kNumberOfControls;
        }

        //	fill out the list with as many objects as requested, which is everything
        if (theNumberItemsToFetch > 0) {
            reinterpret_cast<AudioObjectID*>(outData)[0] = mInputMasterVolumeControlObjectID;
        }
        if (theNumberItemsToFetch > 1) {
            reinterpret_cast<AudioObjectID*>(outData)[1] = mOutputMasterVolumeControlObjectID;
        }

        //	report how much we wrote
        outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioObjectID));
        break;

    case kAudioDevicePropertySafetyOffset:
        //	This property returns the how close to now the HAL can read and write. For
        //	this, device, the value is 0 due to the fact that it always vends silence.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertySafetyOffset for the device");
        *reinterpret_cast<UInt32*>(outData) = 0;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyNominalSampleRate:
        //	This property returns the nominal sample rate of the device. Note that we
        //	only need to take the state lock to get this value.
        {
            ThrowIf(inDataSize < sizeof(Float64), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyNominalSampleRate for the device");

            //	The sample rate is protected by the state lock
            CAMutex::Locker theStateLocker(mStateMutex);

            //	need to lock around fetching the sample rate
            *reinterpret_cast<Float64*>(outData) = static_cast<Float64>(mStreamDescription.mSampleRate);
            outDataSize = sizeof(Float64);
        }
        break;

    case kAudioDevicePropertyAvailableNominalSampleRates:
        //	This returns all nominal sample rates the device supports as an array of
        //	AudioValueRangeStructs. Note that for discrete sampler rates, the range
        //	will have the minimum value equal to the maximum value.

        //	Calculate the number of items that have been requested. Note that this
        //	number is allowed to be smaller than the actual size of the list. In such
        //	case, only that number of items will be returned
        theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioValueRange));

        //	clamp it to the number of items we have
        if (theNumberItemsToFetch > 6) {
            theNumberItemsToFetch = 6;
        }

        //	fill out the return array
        if (theNumberItemsToFetch > 0) {
            ((AudioValueRange*)outData)[0].mMinimum = 22050.0;
            ((AudioValueRange*)outData)[0].mMaximum = 22050.0;
        }
        if (theNumberItemsToFetch > 1) {
            ((AudioValueRange*)outData)[1].mMinimum = 32000.0;
            ((AudioValueRange*)outData)[1].mMaximum = 32000.0;
        }
        if (theNumberItemsToFetch > 2) {
            ((AudioValueRange*)outData)[2].mMinimum = 44100.0;
            ((AudioValueRange*)outData)[2].mMaximum = 44100.0;
        }
        if (theNumberItemsToFetch > 3) {
            ((AudioValueRange*)outData)[3].mMinimum = 48000.0;
            ((AudioValueRange*)outData)[3].mMaximum = 48000.0;
        }
        if (theNumberItemsToFetch > 4) {
            ((AudioValueRange*)outData)[4].mMinimum = 88200.0;
            ((AudioValueRange*)outData)[4].mMaximum = 88200.0;
        }
        if (theNumberItemsToFetch > 5) {
            ((AudioValueRange*)outData)[5].mMinimum = 96000.0;
            ((AudioValueRange*)outData)[5].mMaximum = 96000.0;
        }

        //	report how much we wrote
        outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioValueRange));
        break;

    case kAudioDevicePropertyIsHidden:
        //	This returns whether or not the device is visible to clients.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyIsHidden for the device");
        *reinterpret_cast<UInt32*>(outData) = 0;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioDevicePropertyPreferredChannelsForStereo:
        //	This property returns which two channesl to use as left/right for stereo
        //	data by default. Note that the channel numbers are 1-based.
        ThrowIf(inDataSize < (2 * sizeof(UInt32)), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyPreferredChannelsForStereo for the device");
        ((UInt32*)outData)[0] = 1;
        ((UInt32*)outData)[1] = 2;
        outDataSize = 2 * sizeof(UInt32);
        break;

    case kAudioDevicePropertyPreferredChannelLayout:
        //	This property returns the default AudioChannelLayout to use for the device
        //	by default. For this device, we return a stereo ACL.
        {
            //	calcualte how big the
            UInt32 theACLSize = offsetof(AudioChannelLayout, mChannelDescriptions) + (2 * sizeof(AudioChannelDescription));
            ThrowIf(inDataSize < theACLSize, CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyPreferredChannelLayout for the device");
            ((AudioChannelLayout*)outData)->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
            ((AudioChannelLayout*)outData)->mChannelBitmap = 0;
            ((AudioChannelLayout*)outData)->mNumberChannelDescriptions = 2;
            for (theItemIndex = 0; theItemIndex < 2; ++theItemIndex) {
                ((AudioChannelLayout*)outData)->mChannelDescriptions[theItemIndex].mChannelLabel = kAudioChannelLabel_Left + theItemIndex;
                ((AudioChannelLayout*)outData)->mChannelDescriptions[theItemIndex].mChannelFlags = 0;
                ((AudioChannelLayout*)outData)->mChannelDescriptions[theItemIndex].mCoordinates[0] = 0;
                ((AudioChannelLayout*)outData)->mChannelDescriptions[theItemIndex].mCoordinates[1] = 0;
                ((AudioChannelLayout*)outData)->mChannelDescriptions[theItemIndex].mCoordinates[2] = 0;
            }
            outDataSize = theACLSize;
        }
        break;

    case kAudioDevicePropertyZeroTimeStampPeriod:
        //	This property returns how many frames the HAL should expect to see between
        //	successive sample times in the zero time stamps this device provides.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyZeroTimeStampPeriod for the device");
        *reinterpret_cast<UInt32*>(outData) = mBufferSize;
        outDataSize = sizeof(UInt32);
        break;

    default:
        CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
        break;
    };
}

void UltHub_Device::Device_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Device_GetPropertyData() method.

    switch (inAddress.mSelector) {
    case kAudioDevicePropertyNominalSampleRate:
        //	Changing the sample rate needs to be handled via the RequestConfigChange/PerformConfigChange machinery.
        {
            //	check the arguments
            ThrowIf(inDataSize != sizeof(Float64), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Device_SetPropertyData: wrong size for the data for kAudioDevicePropertyNominalSampleRate");
            //ThrowIf((*((const Float64*)inData) != 44100.0) && (*((const Float64*)inData) != 48000.0), CAException(kAudioHardwareIllegalOperationError), "UltHub_Device::Device_SetPropertyData: unsupported value for kAudioDevicePropertyNominalSampleRate");

            //	we need to lock around getting the current sample rate to compare against the new rate
            UInt64 theOldSampleRate = 0;
            {
                CAMutex::Locker theStateLocker(mStateMutex);
                theOldSampleRate = (UInt64)mStreamDescription.mSampleRate;
            }

            //	make sure that the new value is different than the old value
            UInt64 theNewSampleRate = static_cast<Float64>(*reinterpret_cast<const Float64*>(inData));
            if (theOldSampleRate != theNewSampleRate) {
                Float64* data = new Float64(theNewSampleRate);
                //	we dispatch this so that the change can happen asynchronously
                AudioObjectID theDeviceObjectID = GetObjectID();
                CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
                    UltHub_PlugIn::Host_RequestDeviceConfigurationChange(theDeviceObjectID, kUltraschallHub_SampleRateChange, data);
                });
            }
        }
        break;

    default:
        CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
        break;
    case kAudioObjectPropertySelectorWildcard:
        break;
    };
}

#pragma mark Stream Property Operations

bool UltHub_Device::Stream_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Stream_GetPropertyData() method.

    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioStreamPropertyIsActive:
    case kAudioStreamPropertyDirection:
    case kAudioStreamPropertyTerminalType:
    case kAudioStreamPropertyStartingChannel:
    case kAudioStreamPropertyLatency:
    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
        theAnswer = true;
        break;

    default:
        theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
        break;
    };
    return theAnswer;
}

bool UltHub_Device::Stream_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Stream_GetPropertyData() method.

    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioStreamPropertyDirection:
    case kAudioStreamPropertyTerminalType:
    case kAudioStreamPropertyStartingChannel:
    case kAudioStreamPropertyLatency:
    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
        theAnswer = false;
        break;

    case kAudioStreamPropertyIsActive:
    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
        theAnswer = true;
        break;

    default:
        theAnswer = CAObject::IsPropertySettable(inObjectID, inClientPID, inAddress);
        break;
    };
    return theAnswer;
}

UInt32 UltHub_Device::Stream_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Stream_GetPropertyData() method.

    UInt32 theAnswer = 0;
    switch (inAddress.mSelector) {
    case kAudioStreamPropertyIsActive:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioStreamPropertyDirection:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioStreamPropertyTerminalType:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioStreamPropertyStartingChannel:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioStreamPropertyLatency:
        theAnswer = sizeof(UInt32);
        break;

    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
        theAnswer = sizeof(AudioStreamBasicDescription);
        break;

    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
        theAnswer = 3 * sizeof(AudioStreamRangedDescription);
        break;

    default:
        theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
        break;
    };
    return theAnswer;
}

void UltHub_Device::Stream_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required.
    //	Also, since most of the data that will get returned is static, there are few instances where
    //	it is necessary to lock the state mutex.

    UInt32 theNumberItemsToFetch;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyBaseClass:
        //	The base class for kAudioStreamClassID is kAudioObjectClassID
        ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioObjectPropertyBaseClass for the volume control");
        *reinterpret_cast<AudioClassID*>(outData) = kAudioObjectClassID;
        outDataSize = sizeof(AudioClassID);
        break;

    case kAudioObjectPropertyClass:
        //	Streams are of the class, kAudioStreamClassID
        ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioObjectPropertyClass for the volume control");
        *reinterpret_cast<AudioClassID*>(outData) = kAudioStreamClassID;
        outDataSize = sizeof(AudioClassID);
        break;

    case kAudioObjectPropertyOwner:
        //	The stream's owner is the device object
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioObjectPropertyOwner for the volume control");
        *reinterpret_cast<AudioObjectID*>(outData) = GetObjectID();
        outDataSize = sizeof(AudioObjectID);
        break;

    case kAudioStreamPropertyIsActive:
        //	This property tells the device whether or not the given stream is going to
        //	be used for IO. Note that we need to take the state lock to examine this
        //	value.
        {
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyIsActive for the stream");

            //	lock the state mutex
            CAMutex::Locker theStateLocker(mStateMutex);

            //	return the requested value
            *reinterpret_cast<UInt32*>(outData) = (UInt32)((inAddress.mScope == kAudioObjectPropertyScopeInput) ? mInputStreamIsActive : mOutputStreamIsActive);
            outDataSize = sizeof(UInt32);
        }
        break;

    case kAudioStreamPropertyDirection:
        //	This returns whether the stream is an input stream or an output stream.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyDirection for the stream");
        *reinterpret_cast<UInt32*>(outData) = (inObjectID == mInputStreamObjectID) ? 1 : 0;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioStreamPropertyTerminalType:
        //	This returns a value that indicates what is at the other end of the stream
        //	such as a speaker or headphones, or a microphone. Values for this property
        //	are defined in <CoreAudio/AudioHardwareBase.h>
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyTerminalType for the stream");
        *reinterpret_cast<UInt32*>(outData) = (inObjectID == mInputStreamObjectID) ? kAudioStreamTerminalTypeLine : kAudioStreamTerminalTypeSpeaker;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioStreamPropertyStartingChannel:
        //	This property returns the absolute channel number for the first channel in
        //	the stream. For exmaple, if a device has two output streams with two
        //	channels each, then the starting channel number for the first stream is 1
        //	and ths starting channel number fo the second stream is 3.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyStartingChannel for the stream");
        *reinterpret_cast<UInt32*>(outData) = 1;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioStreamPropertyLatency:
        //	This property returns any additonal presentation latency the stream has.
        ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyStartingChannel for the stream");
        *reinterpret_cast<UInt32*>(outData) = 0;
        outDataSize = sizeof(UInt32);
        break;

    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
        //	This returns the current format of the stream in an AudioStreamBasicDescription.
        //	For devices that don't override the mix operation, the virtual format has to be the
        //	same as the physical format.
        {
            ThrowIf(inDataSize < sizeof(AudioStreamBasicDescription), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyVirtualFormat for the stream");

            //	lock the state mutex
            CAMutex::Locker theStateLocker(mStateMutex);

            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mSampleRate = static_cast<Float64>(mStreamDescription.mSampleRate);
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mFormatID = mStreamDescription.mFormatID;
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mFormatFlags = mStreamDescription.mFormatFlags;
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mBytesPerPacket = mStreamDescription.mBytesPerPacket;
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mFramesPerPacket = mStreamDescription.mFramesPerPacket;
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mBytesPerFrame = mStreamDescription.mBytesPerFrame;
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mChannelsPerFrame = mStreamDescription.mChannelsPerFrame;
            reinterpret_cast<AudioStreamBasicDescription*>(outData)->mBitsPerChannel = mStreamDescription.mBitsPerChannel;
            outDataSize = sizeof(AudioStreamBasicDescription);
        }
        break;

    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats: {
        //	This returns an array of AudioStreamRangedDescriptions that describe what
        //	formats are supported.

        //	lock the state mutex
        CAMutex::Locker theStateLocker(mStateMutex);

        //	Calculate the number of items that have been requested. Note that this
        //	number is allowed to be smaller than the actual size of the list. In such
        //	case, only that number of items will be returned
        theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioStreamRangedDescription));

        //	clamp it to the number of items we have
        if (theNumberItemsToFetch > 3) {
            theNumberItemsToFetch = 3;
        }

        //	fill out the return array
        if (theNumberItemsToFetch > 0) {
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mSampleRate = kAudioStreamAnyRate;
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mFormatID = kAudioFormatLinearPCM;
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mBytesPerPacket = mNumChannels * sizeof(Float32);
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mFramesPerPacket = 1;
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mBytesPerFrame = mNumChannels * sizeof(Float32);
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mChannelsPerFrame = mNumChannels;
            ((AudioStreamRangedDescription*)outData)[0].mFormat.mBitsPerChannel = sizeof(Float32) * 8;
            ((AudioStreamRangedDescription*)outData)[0].mSampleRateRange.mMinimum = 44100.0;
            ((AudioStreamRangedDescription*)outData)[0].mSampleRateRange.mMaximum = 96000.0;
        }
        if (theNumberItemsToFetch > 1) {
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mSampleRate = kAudioStreamAnyRate;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mFormatID = kAudioFormatLinearPCM;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mBytesPerPacket = mNumChannels * 3;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mFramesPerPacket = 1;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mBytesPerFrame = mNumChannels * 3;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mChannelsPerFrame = mNumChannels;
            ((AudioStreamRangedDescription*)outData)[2].mFormat.mBitsPerChannel = 24;
            ((AudioStreamRangedDescription*)outData)[2].mSampleRateRange.mMinimum = 44100.0;
            ((AudioStreamRangedDescription*)outData)[2].mSampleRateRange.mMaximum = 96000.0;
        }
        if (theNumberItemsToFetch > 2) {
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mSampleRate = kAudioStreamAnyRate;
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mFormatID = kAudioFormatLinearPCM;
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mBytesPerPacket = mNumChannels * sizeof(int16_t);
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mFramesPerPacket = 1;
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mBytesPerFrame = mNumChannels * sizeof(int16_t);
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mChannelsPerFrame = mNumChannels;
            ((AudioStreamRangedDescription*)outData)[1].mFormat.mBitsPerChannel = sizeof(int16_t) * 8;
            ((AudioStreamRangedDescription*)outData)[1].mSampleRateRange.mMinimum = 44100.0;
            ((AudioStreamRangedDescription*)outData)[1].mSampleRateRange.mMaximum = 96000.0;
        }

        //	report how much we wrote
        outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioStreamRangedDescription));
        break;
    }
    default:
        CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
        break;
    };
}

void UltHub_Device::Stream_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Stream_GetPropertyData() method.

    switch (inAddress.mSelector) {
    case kAudioStreamPropertyIsActive: {
        //	Changing the active state of a stream doesn't affect IO or change the structure
        //	so we can just save the state and send the notification.
        ThrowIf(inDataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_SetPropertyData: wrong size for the data for kAudioDevicePropertyNominalSampleRate");
        bool theNewIsActive = *reinterpret_cast<const UInt32*>(inData) != 0;

        CAMutex::Locker theStateLocker(mStateMutex);
        if (inObjectID == mInputStreamObjectID) {
            if (mInputStreamIsActive != theNewIsActive) {
                mInputStreamIsActive = theNewIsActive;
            }
        }
        else {
            if (mOutputStreamIsActive != theNewIsActive) {
                mOutputStreamIsActive = theNewIsActive;
            }
        }
    } break;

    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat: {
        //	Changing the stream format needs to be handled via the
        //	RequestConfigChange/PerformConfigChange machinery.
        ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Stream_SetPropertyData: wrong size for the data for kAudioStreamPropertyPhysicalFormat");

        const AudioStreamBasicDescription* theNewFormat = reinterpret_cast<const AudioStreamBasicDescription*>(inData);

        ThrowIf(theNewFormat->mFormatID != kAudioFormatLinearPCM, CAException(kAudioDeviceUnsupportedFormatError),
                "UltHub_Device::Stream_SetPropertyData: unsupported format ID for kAudioStreamPropertyPhysicalFormat");

        ThrowIf(theNewFormat->mChannelsPerFrame != mStreamDescription.mChannelsPerFrame, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported channels per frame for kAudioStreamPropertyPhysicalFormat");

        ThrowIf(theNewFormat->mFramesPerPacket != mStreamDescription.mFramesPerPacket, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported frames per packet for kAudioStreamPropertyPhysicalFormat");

        if (theNewFormat->mFormatFlags == kAudioFormatFlagsNativeFloatPacked) {
            ThrowIf(theNewFormat->mBitsPerChannel != sizeof(Float32) * 8, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported bits per channel for kAudioStreamPropertyPhysicalFormat");
            ThrowIf(theNewFormat->mBytesPerPacket != mNumChannels * sizeof(Float32), CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported bytes per packet for kAudioStreamPropertyPhysicalFormat");
            ThrowIf(theNewFormat->mBytesPerFrame != mNumChannels * sizeof(Float32), CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported frames per packet for kAudioStreamPropertyPhysicalFormat");
        }
        else if (theNewFormat->mFormatFlags == (kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked)) {
            if (theNewFormat->mBitsPerChannel == 24) {
                ThrowIf(theNewFormat->mBytesPerPacket != mNumChannels * 3, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported bytes per packet for kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mBytesPerFrame != mNumChannels * 3, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported frames per packet for kAudioStreamPropertyPhysicalFormat");
            }
            else if (theNewFormat->mBitsPerChannel == sizeof(int16_t) * 8) {
                ThrowIf(theNewFormat->mBytesPerPacket != mNumChannels * sizeof(int16_t), CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported bytes per packet for kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mBytesPerFrame != mNumChannels * sizeof(int16_t), CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported frames per packet for kAudioStreamPropertyPhysicalFormat");
            }
            else {
                ThrowIf(true, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported bits per channel for kAudioStreamPropertyPhysicalFormat");
            }
        }
        else {
            ThrowIf(true, CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported format flags for kAudioStreamPropertyPhysicalFormat");
        }

        //  ThrowIf((theNewFormat->mSampleRate != 44100.0) && (theNewFormat->mSampleRate != 48000.0), CAException(kAudioDeviceUnsupportedFormatError), "UltHub_Device::Stream_SetPropertyData: unsupported sample rate for kAudioStreamPropertyPhysicalFormat");

        bool isChanged = false;
        //	we need to lock around getting the current stream description to compare against the new one
        {
            CAMutex::Locker theStateLocker(mStateMutex);
            isChanged = (theNewFormat->mSampleRate != mStreamDescription.mSampleRate
                         || theNewFormat->mFormatFlags != mStreamDescription.mFormatFlags
                         || theNewFormat->mBytesPerPacket != mStreamDescription.mBytesPerPacket
                         || theNewFormat->mBytesPerFrame != mStreamDescription.mBytesPerFrame
                         || theNewFormat->mBitsPerChannel != mStreamDescription.mBitsPerChannel);
        }

        if (isChanged) {
            AudioStreamBasicDescription* format = new AudioStreamBasicDescription(*theNewFormat);
            //	we dispatch this so that the change can happen asynchronously
            AudioObjectID theDeviceObjectID = GetObjectID();
            CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
                UltHub_PlugIn::Host_RequestDeviceConfigurationChange(theDeviceObjectID, kUltraschallHub_StreamFormatChange, format);
            });
        }
    } break;

    default:
        CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
        break;
    };
}

#pragma mark Control Property Operations

bool UltHub_Device::Control_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Control_GetPropertyData() method.

    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioControlPropertyScope:
    case kAudioControlPropertyElement:
    case kAudioLevelControlPropertyScalarValue:
    case kAudioLevelControlPropertyDecibelValue:
    case kAudioLevelControlPropertyDecibelRange:
    case kAudioLevelControlPropertyConvertScalarToDecibels:
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
        theAnswer = true;
        break;

    default:
        theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
        break;
    };
    return theAnswer;
}

bool UltHub_Device::Control_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Control_GetPropertyData() method.

    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioControlPropertyScope:
    case kAudioControlPropertyElement:
    case kAudioLevelControlPropertyDecibelRange:
    case kAudioLevelControlPropertyConvertScalarToDecibels:
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
        theAnswer = false;
        break;

    case kAudioLevelControlPropertyScalarValue:
    case kAudioLevelControlPropertyDecibelValue:
        theAnswer = true;
        break;

    default:
        theAnswer = CAObject::IsPropertySettable(inObjectID, inClientPID, inAddress);
        break;
    };
    return theAnswer;
}

UInt32 UltHub_Device::Control_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Control_GetPropertyData() method.

    UInt32 theAnswer = 0;
    switch (inAddress.mSelector) {
    case kAudioControlPropertyScope:
        theAnswer = sizeof(AudioObjectPropertyScope);
        break;

    case kAudioControlPropertyElement:
        theAnswer = sizeof(AudioObjectPropertyElement);
        break;

    case kAudioLevelControlPropertyScalarValue:
        theAnswer = sizeof(Float32);
        break;

    case kAudioLevelControlPropertyDecibelValue:
        theAnswer = sizeof(Float32);
        break;

    case kAudioLevelControlPropertyDecibelRange:
        theAnswer = sizeof(AudioValueRange);
        break;

    case kAudioLevelControlPropertyConvertScalarToDecibels:
        theAnswer = sizeof(Float32);
        break;

    case kAudioLevelControlPropertyConvertDecibelsToScalar:
        theAnswer = sizeof(Float32);
        break;

    default:
        theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
        break;
    };
    return theAnswer;
}

void UltHub_Device::Control_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required.
    //	Also, since most of the data that will get returned is static, there are few instances where
    //	it is necessary to lock the state mutex.

    Float32 theVolumeValue;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyBaseClass:
        //	The base class for kAudioVolumeControlClassID is kAudioLevelControlClassID
        ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioObjectPropertyBaseClass for the volume control");
        *reinterpret_cast<AudioClassID*>(outData) = kAudioLevelControlClassID;
        outDataSize = sizeof(AudioClassID);
        break;

    case kAudioObjectPropertyClass:
        //	Volume controls are of the class, kAudioVolumeControlClassID
        ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioObjectPropertyClass for the volume control");
        *reinterpret_cast<AudioClassID*>(outData) = kAudioVolumeControlClassID;
        outDataSize = sizeof(AudioClassID);
        break;

    case kAudioObjectPropertyOwner:
        //	The control's owner is the device object
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioObjectPropertyOwner for the volume control");
        *reinterpret_cast<AudioObjectID*>(outData) = GetObjectID();
        outDataSize = sizeof(AudioObjectID);
        break;

    case kAudioControlPropertyScope:
        //	This property returns the scope that the control is attached to.
        ThrowIf(inDataSize < sizeof(AudioObjectPropertyScope), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioControlPropertyScope for the volume control");
        *reinterpret_cast<AudioObjectPropertyScope*>(outData) = (inObjectID == mInputMasterVolumeControlObjectID) ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput;
        outDataSize = sizeof(AudioObjectPropertyScope);
        break;

    case kAudioControlPropertyElement:
        //	This property returns the element that the control is attached to.
        ThrowIf(inDataSize < sizeof(AudioObjectPropertyElement), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioControlPropertyElement for the volume control");
        *reinterpret_cast<AudioObjectPropertyElement*>(outData) = kAudioObjectPropertyElementMaster;
        outDataSize = sizeof(AudioObjectPropertyElement);
        break;

    case kAudioLevelControlPropertyScalarValue:
        //	This returns the value of the control in the normalized range of 0 to 1.
        {
            ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyScalarValue for the volume control");
            CAMutex::Locker theStateLocker(mStateMutex);
            if (inObjectID == mInputMasterVolumeControlObjectID) {
                *reinterpret_cast<Float32*>(outData) = mMasterInputVolume;
            }
            else {
                *reinterpret_cast<Float32*>(outData) = mMasterOutputVolume;
            }
            outDataSize = sizeof(Float32);
        }
        break;

    case kAudioLevelControlPropertyDecibelValue:
        //	This returns the dB value of the control.
        {
            ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelValue for the volume control");
            CAMutex::Locker theStateLocker(mStateMutex);
            if (inObjectID == mInputMasterVolumeControlObjectID) {
                *reinterpret_cast<Float32*>(outData) = mVolumeCurve.ConvertScalarToDB(mMasterInputVolume);
            }
            else {
                *reinterpret_cast<Float32*>(outData) = mVolumeCurve.ConvertScalarToDB(mMasterOutputVolume);
            }
            outDataSize = sizeof(Float32);
        }
        break;

    case kAudioLevelControlPropertyDecibelRange:
        //	This returns the dB range of the control.
        ThrowIf(inDataSize < sizeof(AudioValueRange), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelRange for the volume control");
        reinterpret_cast<AudioValueRange*>(outData)->mMinimum = mVolumeCurve.GetMinimumDB();
        reinterpret_cast<AudioValueRange*>(outData)->mMaximum = mVolumeCurve.GetMaximumDB();
        outDataSize = sizeof(AudioValueRange);
        break;

    case kAudioLevelControlPropertyConvertScalarToDecibels:
        //	This takes the scalar value in outData and converts it to dB.
        ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelValue for the volume control");

        //	clamp the value to be between 0 and 1
        theVolumeValue = *reinterpret_cast<Float32*>(outData);
        theVolumeValue = std::min(1.0f, std::max(0.0f, theVolumeValue));

        //	do the conversion
        *reinterpret_cast<Float32*>(outData) = mVolumeCurve.ConvertScalarToDB(theVolumeValue);

        //	report how much we wrote
        outDataSize = sizeof(Float32);
        break;

    case kAudioLevelControlPropertyConvertDecibelsToScalar:
        //	This takes the dB value in outData and converts it to scalar.
        ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelValue for the volume control");

        //	clamp the value to be between kVolume_MinDB and kVolume_MaxDB
        theVolumeValue = *reinterpret_cast<Float32*>(outData);
        theVolumeValue = std::min(kUltraschallHub_Control_MaxDbVolumeValue, std::max(kUltraschallHub_Control_MinDBVolumeValue, theVolumeValue));

        //	do the conversion
        *reinterpret_cast<Float32*>(outData) = mVolumeCurve.ConvertDBToScalar(theVolumeValue);

        //	report how much we wrote
        outDataSize = sizeof(Float32);
        break;

    default:
        CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
        break;
    };
}

void UltHub_Device::Control_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Control_GetPropertyData() method.

    bool sendNotifications = false;
    Float32 theNewVolumeValue;
    switch (inAddress.mSelector) {
    case kAudioLevelControlPropertyScalarValue:
        //	For the scalar volume, we clamp the new value to [0, 1]. Note that if this
        //	value changes, it implies that the dB value changed too.
        {
            ThrowIf(inDataSize != sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_SetPropertyData: wrong size for the data for kAudioLevelControlPropertyScalarValue");
            theNewVolumeValue = *((const Float32*)inData);
            theNewVolumeValue = std::min(1.0f, std::max(0.0f, theNewVolumeValue));
            CAMutex::Locker theStateLocker(mStateMutex);
            if (inObjectID == mInputMasterVolumeControlObjectID) {
                mMasterInputVolume = theNewVolumeValue;
            }
            else {
                mMasterOutputVolume = theNewVolumeValue;
            }
            sendNotifications = true;
        }
        break;

    case kAudioLevelControlPropertyDecibelValue:
        //	For the dB value, we first convert it to a scalar value since that is how
        //	the value is tracked. Note that if this value changes, it implies that the
        //	scalar value changes as well.
        {
            ThrowIf(inDataSize != sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "UltHub_Device::Control_SetPropertyData: wrong size for the data for kAudioLevelControlPropertyScalarValue");
            theNewVolumeValue = *((const Float32*)inData);
            theNewVolumeValue = std::min(kUltraschallHub_Control_MaxDbVolumeValue, std::max(kUltraschallHub_Control_MinDBVolumeValue, theNewVolumeValue));
            theNewVolumeValue = mVolumeCurve.ConvertDBToScalar(theNewVolumeValue);
            CAMutex::Locker theStateLocker(mStateMutex);
            if (inObjectID == mInputMasterVolumeControlObjectID) {
                mMasterInputVolume = theNewVolumeValue;
            }
            else {
                mMasterOutputVolume = theNewVolumeValue;
            }
            sendNotifications = true;
        }
        break;

    default:
        CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
        break;
    };

    if (sendNotifications) {
        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            AudioObjectPropertyAddress theChangedProperties[] = { { kAudioLevelControlPropertyScalarValue, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster }, { kAudioLevelControlPropertyDecibelValue, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster } };
            UltHub_PlugIn::Host_PropertiesChanged(inObjectID, 2, theChangedProperties);
        });
    }
}

#pragma mark IO Operations

void UltHub_Device::StartIO()
{
    //	Starting/Stopping IO needs to be reference counted due to the possibility of multiple clients starting IO
    CAMutex::Locker theStateLocker(mStateMutex);

    //	make sure we can start
    ThrowIf(mStartCount == UINT64_MAX, CAException(kAudioHardwareIllegalOperationError), "UltHub_Device::StartIO: failed to start because the ref count was maxxed out already");

    //	we only tell the hardware to start if this is the first time IO has been started
    if (mStartCount == 0) {
        mStreamRingBuffer.Deallocate();
        mStreamRingBuffer.Allocate(mStreamDescription.mChannelsPerFrame, mStreamDescription.mBytesPerFrame, mBufferSize);

        mTicksPerFrame = CAHostTimeBase::GetFrequency() / mStreamDescription.mSampleRate;
        mAnchorHostTime = CAHostTimeBase::GetCurrentTime();
        mTimeline++;
        if (mTimeline == UINT64_MAX)
            mTimeline = 0;
    }
    ++mStartCount;
}

void UltHub_Device::StopIO()
{
    //	Starting/Stopping IO needs to be reference counted due to the possibility of multiple clients starting IO
    CAMutex::Locker theStateLocker(mStateMutex);

    //	we tell the hardware to stop if this is the last stop call
    if (mStartCount == 1) {
        mStartCount = 0;
    }
    else if (mStartCount > 1) {
        --mStartCount;
    }
}

void UltHub_Device::GetZeroTimeStamp(Float64& outSampleTime, UInt64& outHostTime, UInt64& outSeed) const
{
    static UInt64 mNumberTimeStamps = 0;
    static UInt64 currentTimeLine = 0;

    if (mTimeline != currentTimeLine) {
        currentTimeLine = mTimeline;
        mNumberTimeStamps = 0;
    }

    //	accessing the mapped memory requires holding the IO mutex
    CAMutex::Locker theIOLocker(mIOMutex);

    UInt64 theCurrentHostTime;
    Float64 theHostTicksPerRingBuffer;
    Float64 theHostTickOffset;
    UInt64 theNextHostTime;

    theCurrentHostTime = CAHostTimeBase::GetCurrentTime();

    //	calculate the next host time
    theHostTicksPerRingBuffer = mTicksPerFrame * mBufferSize;
    theHostTickOffset = ((Float64)(mNumberTimeStamps + 1)) * theHostTicksPerRingBuffer;
    theNextHostTime = mAnchorHostTime + ((UInt64)theHostTickOffset);
    //	go to the next time if the next host time is less than the current time
    if (theNextHostTime <= theCurrentHostTime) {
        ++mNumberTimeStamps;
    }

    //	set the return values
    outSampleTime = mNumberTimeStamps * mBufferSize;
    outHostTime = mAnchorHostTime + (mNumberTimeStamps * theHostTicksPerRingBuffer);
    outSeed = mTimeline;
}

void UltHub_Device::WillDoIOOperation(UInt32 inOperationID, bool& outWillDo, bool& outWillDoInPlace) const
{
    switch (inOperationID) {
    case kAudioServerPlugInIOOperationReadInput:
    case kAudioServerPlugInIOOperationWriteMix:
        outWillDo = true;
        outWillDoInPlace = true;
        break;

    case kAudioServerPlugInIOOperationThread:
    case kAudioServerPlugInIOOperationCycle:
    case kAudioServerPlugInIOOperationConvertInput:
    case kAudioServerPlugInIOOperationProcessInput:
    case kAudioServerPlugInIOOperationProcessOutput:
    case kAudioServerPlugInIOOperationMixOutput:
    case kAudioServerPlugInIOOperationProcessMix:
    case kAudioServerPlugInIOOperationConvertMix:
    default:
        outWillDo = false;
        outWillDoInPlace = true;
        break;
    };
}

void UltHub_Device::BeginIOOperation(UInt32 /*inOperationID*/, UInt32 /*inIOBufferFrameSize*/, const AudioServerPlugInIOCycleInfo& inIOCycleInfo)
{
}

void UltHub_Device::DoIOOperation(AudioObjectID /*inStreamObjectID*/, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo& inIOCycleInfo, void* ioMainBuffer, void* /*ioSecondaryBuffer*/)
{
    switch (inOperationID) {
    case kAudioServerPlugInIOOperationReadInput:
        ReadInputData(inIOBufferFrameSize, inIOCycleInfo.mInputTime.mSampleTime, ioMainBuffer);
        break;

    case kAudioServerPlugInIOOperationWriteMix:
        WriteOutputData(inIOBufferFrameSize, inIOCycleInfo.mOutputTime.mSampleTime, ioMainBuffer);
        break;
    default:
        break;
    };
}

void UltHub_Device::EndIOOperation(UInt32 /*inOperationID*/, UInt32 /*inIOBufferFrameSize*/, const AudioServerPlugInIOCycleInfo& inIOCycleInfo)
{
}

void UltHub_Device::ReadInputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void* outBuffer)
{
    //	we need to be holding the IO lock to do this
    CAMutex::Locker theIOLocker(mIOMutex);

    AudioBuffer buffer;
    buffer.mDataByteSize = inIOBufferFrameSize * mStreamDescription.mBytesPerFrame;
    buffer.mNumberChannels = mStreamDescription.mChannelsPerFrame;
    buffer.mData = outBuffer;

    AudioBufferList* bufferList = new AudioBufferList();
    bufferList->mNumberBuffers = 1;
    bufferList->mBuffers[0] = buffer;

    CARingBufferError error = mStreamRingBuffer.Fetch(bufferList, inIOBufferFrameSize, inSampleTime);

    if (error != kCARingBufferError_OK) {
        if (error == kCARingBufferError_CPUOverload) {
            DebugMessage("UltHub_Device::ReadInputData: kCARingBufferError_CPUOverload");
        }
        else if (error == kCARingBufferError_TooMuch) {
            DebugMessage("UltHub_Device::ReadInputData: kCARingBufferError_TooMuch");
        }
        else {
            DebugMessage("UltHub_Device::ReadInputData: RingBufferError Unknown");
        }
    }
}

void UltHub_Device::WriteOutputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void* inBuffer)
{
    //	we need to be holding the IO lock to do this
    CAMutex::Locker theIOLocker(mIOMutex);

    AudioBuffer buffer;
    buffer.mDataByteSize = inIOBufferFrameSize * mStreamDescription.mBytesPerFrame;
    buffer.mNumberChannels = mStreamDescription.mChannelsPerFrame;
    buffer.mData = inBuffer;

    AudioBufferList* bufferList = new AudioBufferList();
    bufferList->mNumberBuffers = 1;
    bufferList->mBuffers[0] = buffer;

    CARingBufferError error = mStreamRingBuffer.Store(bufferList, inIOBufferFrameSize, inSampleTime);
    if (error != kCARingBufferError_OK) {
        if (error == kCARingBufferError_CPUOverload) {
            DebugMessage("UltHub_Device::ReadInputData: kCARingBufferError_CPUOverload");
        }
        else if (error == kCARingBufferError_TooMuch) {
            DebugMessage("UltHub_Device::ReadInputData: kCARingBufferError_TooMuch");
        }
        else {
            DebugMessage("UltHub_Device::ReadInputData: RingBufferError Unknown");
        }
    }
}

#pragma mark Implementation

void UltHub_Device::PerformConfigChange(UInt64 inChangeAction, void* inChangeInfo)
{

    if (inChangeAction == kUltraschallHub_StreamFormatChange) {
        AudioStreamBasicDescription* theNewFormat = reinterpret_cast<AudioStreamBasicDescription*>(inChangeInfo);
        ThrowIfNULL(theNewFormat, CAException(kAudioHardwareIllegalOperationError), "UltHub_Device::PerformConfigChange: illegal data for kUltraschallHub_DeviceConfigurationChange");

        // we need to be holding the IO and State lock to do this
        CAMutex::Locker theStateLocker(mStateMutex);
        CAMutex::Locker theIOLocker(mIOMutex);

        mStreamDescription.mSampleRate = theNewFormat->mSampleRate;
        mStreamDescription.mFormatFlags = theNewFormat->mFormatFlags;
        mStreamDescription.mBytesPerPacket = theNewFormat->mBytesPerPacket;
        mStreamDescription.mBytesPerFrame = theNewFormat->mBytesPerFrame;
        mStreamDescription.mBitsPerChannel = theNewFormat->mBitsPerChannel;

        delete theNewFormat;
    }
    else if (inChangeAction == kUltraschallHub_SampleRateChange) {
        Float64* theNewSampleRate = reinterpret_cast<Float64*>(inChangeInfo);
        ThrowIfNULL(theNewSampleRate, CAException(kAudioHardwareIllegalOperationError), "UltHub_Device::PerformConfigChange: illegal data for kUltraschallHub_DeviceConfigurationChange");

        // we need to be holding the IO and State lock to do this
        CAMutex::Locker theStateLocker(mStateMutex);
        CAMutex::Locker theIOLocker(mIOMutex);

        mStreamDescription.mSampleRate = *theNewSampleRate;
        delete theNewSampleRate;
    }
}

void UltHub_Device::AbortConfigChange(UInt64 /*inChangeAction*/, void* /*inChangeInfo*/)
{

    //	this device doesn't need to do anything special if a change request gets aborted
}