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

#include "Device.h"

#include "PlugIn.h"
#include "AudioHubTypes.h"
#include <Accelerate/Accelerate.h>
#include "CADispatchQueue.h"
#include "CAException.h"

#pragma mark Construction/Destruction

Device::Device(AudioObjectID inObjectID, SInt16 numChannels, AudioObjectID owner)
        : CAObject(inObjectID, kAudioDeviceClassID, kAudioObjectClassID, owner),
          mStateMutex(new CAMutex("Hub State")),
          mIOMutex(new CAMutex("Hub IO")),
          mStartCount(0),
          mRingBufferSize(1024 * 8),
          mDeviceUID("Hub:0"),
          mInputStreamObjectID(CAObjectMap::GetNextObjectID()),
          mInputStreamIsActive(true),
          mOutputStreamObjectID(CAObjectMap::GetNextObjectID()),
          mOutputStreamIsActive(true),
          mInputMasterVolumeControlObjectID(CAObjectMap::GetNextObjectID()),
          mInputMasterVolumeControlRawValueShadow(kHub_Control_MinRawVolumeValue),
          mOutputMasterVolumeControlObjectID(CAObjectMap::GetNextObjectID()),
          mOutputMasterVolumeControlRawValueShadow(kHub_Control_MinRawVolumeValue),
          mVolumeCurve(),
          mTimeline(0),
          mMasterInputVolume(1),
          mMasterOutputVolume(1) {
    //  put the device info in the list
    mStreamDescriptions.push_back(CAStreamBasicDescription(44100.0, numChannels, CAStreamBasicDescription::kPCMFormatFloat32, true));
    mStreamDescriptions.push_back(CAStreamBasicDescription(48000.0, numChannels, CAStreamBasicDescription::kPCMFormatFloat32, true));
    mStreamDescriptions.push_back(CAStreamBasicDescription(96000.0, numChannels, CAStreamBasicDescription::kPCMFormatFloat32, true));

    mStreamDescription = mStreamDescriptions[1];

    //	Setup the volume curve with the one range
    mVolumeCurve.AddRange(kHub_Control_MinRawVolumeValue, kHub_Control_MaxRawVolumeValue, kHub_Control_MinDBVolumeValue, kHub_Control_MaxDbVolumeValue);
}

void Device::Activate() {
    //	map the subobject IDs to this object
    CAObjectMap::MapObject(mInputStreamObjectID, this);
    CAObjectMap::MapObject(mOutputStreamObjectID, this);
    CAObjectMap::MapObject(mInputMasterVolumeControlObjectID, this);
    CAObjectMap::MapObject(mOutputMasterVolumeControlObjectID, this);

    //	call the super-class, which just marks the object as active
    CAObject::Activate();
}

void Device::Deactivate() {
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

Device::~Device() {
    mRingBuffer.Deallocate();
    delete mStateMutex;
    delete mIOMutex;
    mStateMutex = nullptr;
    mIOMutex = nullptr;
}

#pragma mark Property Operations

bool Device::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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
    else {Throw(CAException(kAudioHardwareBadObjectError));
    }
    return theAnswer;
}

bool Device::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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
    else {Throw(CAException(kAudioHardwareBadObjectError));
    }
    return theAnswer;
}

UInt32 Device::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const {
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
    else {Throw(CAException(kAudioHardwareBadObjectError));
    }
    return theAnswer;
}

void Device::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const {
    if (inObjectID == mObjectID) {
        Device_GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        Stream_GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        Control_GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
    }
    else {Throw(CAException(kAudioHardwareBadObjectError));
    }
}

void Device::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    if (inObjectID == mObjectID) {
        Device_SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
    }
    else if ((inObjectID == mInputStreamObjectID) || (inObjectID == mOutputStreamObjectID)) {
        Stream_SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
    }
    else if ((inObjectID == mInputMasterVolumeControlObjectID) || (inObjectID == mOutputMasterVolumeControlObjectID)) {
        Control_SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
    }
    else {Throw(CAException(kAudioHardwareBadObjectError));
    }
}

#pragma mark Device Property Operations

bool Device::Device_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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
        case kAudioDevicePropertyIcon:
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

bool Device::Device_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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
        case kAudioDevicePropertyIcon:
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

UInt32 Device::Device_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const {
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
            theAnswer = 3 * sizeof(AudioValueRange);
            break;

        case kAudioDevicePropertyIcon:
            theAnswer = sizeof(CFURLRef);
            break;

        case kAudioDevicePropertyIsHidden:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyPreferredChannelsForStereo:
            theAnswer = 2 * sizeof(UInt32);
            break;

        case kAudioDevicePropertyPreferredChannelLayout:
            theAnswer = offsetof(AudioChannelLayout, mChannelDescriptions) + (mStreamDescription.mChannelsPerFrame * sizeof(AudioChannelDescription));
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

void Device::Device_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const {
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
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer for the device");
            *reinterpret_cast<CFStringRef *>(outData) = mDeviceName.CopyCFString();
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyManufacturer:
            //	This is the human readable name of the maker of the plug-in. Note that in this case
            //	we return a value that is a key into the localizable strings in this bundle. This
            //	allows us to return a localized name for the manufacturer.
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer for the device");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubManufacturer;
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyOwnedObjects:
            //	Calculate the number of items that have been requested. Note that this
            //	number is allowed to be smaller than the actual size of the list. In such
            //	case, only that number of items will be returned
            theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioObjectID));

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
                        reinterpret_cast<AudioObjectID *>(outData)[0] = mInputStreamObjectID;
                    }
                    if (theNumberItemsToFetch > 1) {
                        reinterpret_cast<AudioObjectID *>(outData)[1] = mOutputStreamObjectID;
                    }
                    if (theNumberItemsToFetch > 2) {
                        reinterpret_cast<AudioObjectID *>(outData)[2] = mInputMasterVolumeControlObjectID;
                    }
                    if (theNumberItemsToFetch > 3) {
                        reinterpret_cast<AudioObjectID *>(outData)[3] = mOutputMasterVolumeControlObjectID;
                    }
                    break;

                case kAudioObjectPropertyScopeInput:
                    //	input scope means just the objects on the input side
                    if (theNumberItemsToFetch > kNumberOfInputSubObjects) {
                        theNumberItemsToFetch = kNumberOfInputSubObjects;
                    }

                    //	fill out the list with the right objects
                    if (theNumberItemsToFetch > 0) {
                        reinterpret_cast<AudioObjectID *>(outData)[0] = mInputStreamObjectID;
                    }
                    if (theNumberItemsToFetch > 1) {
                        reinterpret_cast<AudioObjectID *>(outData)[1] = mInputMasterVolumeControlObjectID;
                    }
                    break;

                case kAudioObjectPropertyScopeOutput:
                    //	output scope means just the objects on the output side
                    if (theNumberItemsToFetch > kNumberOfOutputSubObjects) {
                        theNumberItemsToFetch = kNumberOfOutputSubObjects;
                    }

                    //	fill out the list with the right objects
                    if (theNumberItemsToFetch > 0) {
                        reinterpret_cast<AudioObjectID *>(outData)[0] = mOutputStreamObjectID;
                    }
                    if (theNumberItemsToFetch > 1) {
                        reinterpret_cast<AudioObjectID *>(outData)[1] = mOutputMasterVolumeControlObjectID;
                    }
                    break;
                case kAudioObjectPropertyScopePlayThrough:
                    break;
                case kAudioObjectPropertyScopeWildcard:
                    break;
            };

            //	report how much we wrote
            outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioObjectID));
            break;

        case kAudioDevicePropertyDeviceUID:
            //	This is a CFString that is a persistent token that can identify the same
            //	audio device across boot sessions. Note that two instances of the same
            //	device must have different values for this property.
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceUID for the device");
            *reinterpret_cast<CFStringRef *>(outData) = mDeviceUID.CopyCFString();
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyModelUID:
            //	This is a CFString that is a persistent token that can identify audio
            //	devices that are the same kind of device. Note that two instances of the
            //	save device must have the same value for this property.
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyModelUID for the device");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubDeviceModelUID;
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyTransportType:
            //	This value represents how the device is attached to the system. This can be
            //	any 32 bit integer, but common values for this property are defined in
            //	<CoreAudio/AudioHardwareBase.h>
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyTransportType for the device");
            *reinterpret_cast<UInt32 *>(outData) = kAudioDeviceTransportTypeVirtual;
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
            theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioObjectID));

            //	we only have the one device...
            if (theNumberItemsToFetch > 1) {
                theNumberItemsToFetch = 1;
            }

            //	Write the devices' object IDs into the return value
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID *>(outData)[0] = GetObjectID();
            }

            //	report how much we wrote
            outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioObjectID));
            break;

        case kAudioDevicePropertyClockDomain:
            //	This property allows the device to declare what other devices it is
            //	synchronized with in hardware. The way it works is that if two devices have
            //	the same value for this property and the value is not zero, then the two
            //	devices are synchronized in hardware. Note that a device that either can't
            //	be synchronized with others or doesn't know should return 0 for this
            //	property.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyClockDomain for the device");
            *reinterpret_cast<UInt32 *>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceIsAlive:
            //	This property returns whether or not the device is alive. Note that it is
            //	note uncommon for a device to be dead but still momentarily availble in the
            //	device list. In the case of this device, it will always be alive.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceIsAlive for the device");
            *reinterpret_cast<UInt32 *>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceIsRunning:
            //	This property returns whether or not IO is running for the device.
        {
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceIsRunning for the device");

            //	The IsRunning state is protected by the state lock
            CAMutex::Locker theStateLocker(mStateMutex);

            //	return the state and how much data we are touching
            *reinterpret_cast<UInt32 *>(outData) = (UInt32) (mStartCount > 0);
            outDataSize = sizeof(UInt32);
        }
            break;

        case kAudioDevicePropertyDeviceCanBeDefaultDevice:
            //	This property returns whether or not the device wants to be able to be the
            //	default device for content. This is the device that iTunes and QuickTime
            //	will use to play their content on and FaceTime will use as it's microhphone.
            //	Nearly all devices should allow for this.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceCanBeDefaultDevice for the device");
            *reinterpret_cast<UInt32 *>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
            //	This property returns whether or not the device wants to be the system
            //	default device. This is the device that is used to play interface sounds and
            //	other incidental or UI-related sounds on. Most devices should allow this
            //	although devices with lots of latency may not want to.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyDeviceCanBeDefaultSystemDevice for the device");
            *reinterpret_cast<UInt32 *>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyLatency:
            //	This property returns the presentation latency of the device.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyLatency for the device");
            *reinterpret_cast<UInt32 *>(outData) = (inAddress.mScope == kAudioObjectPropertyScopeInput) ? mLatencyInput : mLatencyOutput;;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyStreams:
            //	Calculate the number of items that have been requested. Note that this
            //	number is allowed to be smaller than the actual size of the list. In such
            //	case, only that number of items will be returned
            theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioObjectID));

            //	Note that what is returned here depends on the scope requested.
            switch (inAddress.mScope) {
                case kAudioObjectPropertyScopeGlobal:
                    //	global scope means return all streams
                    if (theNumberItemsToFetch > kNumberOfStreams) {
                        theNumberItemsToFetch = kNumberOfStreams;
                    }

                    //	fill out the list with as many objects as requested
                    if (theNumberItemsToFetch > 0) {
                        reinterpret_cast<AudioObjectID *>(outData)[0] = mInputStreamObjectID;
                    }
                    if (theNumberItemsToFetch > 1) {
                        reinterpret_cast<AudioObjectID *>(outData)[1] = mOutputStreamObjectID;
                    }
                    break;

                case kAudioObjectPropertyScopeInput:
                    //	input scope means just the objects on the input side
                    if (theNumberItemsToFetch > kNumberOfInputStreams) {
                        theNumberItemsToFetch = kNumberOfInputStreams;
                    }

                    //	fill out the list with as many objects as requested
                    if (theNumberItemsToFetch > 0) {
                        reinterpret_cast<AudioObjectID *>(outData)[0] = mInputStreamObjectID;
                    }
                    break;

                case kAudioObjectPropertyScopeOutput:
                    //	output scope means just the objects on the output side
                    if (theNumberItemsToFetch > kNumberOfOutputStreams) {
                        theNumberItemsToFetch = kNumberOfOutputStreams;
                    }

                    //	fill out the list with as many objects as requested
                    if (theNumberItemsToFetch > 0) {
                        reinterpret_cast<AudioObjectID *>(outData)[0] = mOutputStreamObjectID;
                    }
                    break;
                case kAudioObjectPropertyScopePlayThrough:
                    break;
                case kAudioObjectPropertyScopeWildcard:
                    break;
            };

            //	report how much we wrote
            outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioObjectID));
            break;

        case kAudioObjectPropertyControlList:
            //	Calculate the number of items that have been requested. Note that this
            //	number is allowed to be smaller than the actual size of the list. In such
            //	case, only that number of items will be returned
            theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioObjectID));
            if (theNumberItemsToFetch > kNumberOfControls) {
                theNumberItemsToFetch = kNumberOfControls;
            }

            //	fill out the list with as many objects as requested, which is everything
            if (theNumberItemsToFetch > 0) {
                reinterpret_cast<AudioObjectID *>(outData)[0] = mInputMasterVolumeControlObjectID;
            }
            if (theNumberItemsToFetch > 1) {
                reinterpret_cast<AudioObjectID *>(outData)[1] = mOutputMasterVolumeControlObjectID;
            }

            //	report how much we wrote
            outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioObjectID));
            break;

        case kAudioDevicePropertySafetyOffset:
            //	This property returns the how close to now the HAL can read and write.
            //  A UInt32 whose value indicates the number for frames in ahead (for output)
            //  or behind (for input the current hardware position that is safe to do IO.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertySafetyOffset for the device");
            *reinterpret_cast<UInt32 *>(outData) = offset_to_uint32((inObjectID == mInputStreamObjectID) ? mSafetyOffsetInput : mSafetyOffsetOutput);
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyNominalSampleRate:
            //	This property returns the nominal sample rate of the device. Note that we
            //	only need to take the state lock to get this value.
        {
            ThrowIf(inDataSize < sizeof(Float64), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyNominalSampleRate for the device");

            //	The sample rate is protected by the state lock
            CAMutex::Locker theStateLocker(mStateMutex);

            //	need to lock around fetching the sample rate
            *reinterpret_cast<Float64 *>(outData) = static_cast<Float64>(mStreamDescription.mSampleRate);
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
            theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioValueRange));

            //	clamp it to the number of items we have
            if (theNumberItemsToFetch > 3) {
                theNumberItemsToFetch = 3;
            }

            //	fill out the return array
            if (theNumberItemsToFetch > 0) {
                ((AudioValueRange *) outData)[0].mMinimum = 44100.0;
                ((AudioValueRange *) outData)[0].mMaximum = 44100.0;
            }
            if (theNumberItemsToFetch > 1) {
                ((AudioValueRange *) outData)[1].mMinimum = 48000.0;
                ((AudioValueRange *) outData)[1].mMaximum = 48000.0;
            }
            if (theNumberItemsToFetch > 2) {
                ((AudioValueRange *) outData)[2].mMinimum = 96000.0;
                ((AudioValueRange *) outData)[2].mMaximum = 96000.0;
            }

            //	report how much we wrote
            outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioValueRange));
            break;

        case kAudioDevicePropertyIcon: {
            //	This is a CFURL that points to the device's Icon in the plug-in's resource bundle.
            ThrowIf(inDataSize < sizeof(CFURLRef), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyIcon for the device");
            CFBundleRef theBundle = CFBundleGetBundleWithIdentifier(kAudioHubBundleIdentifier);
            ThrowIf(theBundle == NULL, CAException(kAudioHardwareUnspecifiedError), "Device::Device_GetPropertyData:: could not get the plug-in bundle for kAudioDevicePropertyIcon");
            CFURLRef theURL = CFBundleCopyResourceURL(theBundle, CFSTR("Device.icns"), NULL, NULL);
            ThrowIf(theURL == NULL, CAException(kAudioHardwareUnspecifiedError), "Device::Device_GetPropertyData:: could not get the URL for kAudioDevicePropertyIcon");
            *((CFURLRef *) outData) = theURL;
            outDataSize = sizeof(CFURLRef);
        }
            break;

        case kAudioDevicePropertyIsHidden:
            //	This returns whether or not the device is visible to clients.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyIsHidden for the device");
            *reinterpret_cast<UInt32 *>(outData) = isHidden;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyPreferredChannelsForStereo: {
            //	This property returns which two channesl to use as left/right for stereo
            //	data by default. Note that the channel numbers are 1-based.
            ThrowIf(inDataSize < (2 * sizeof(UInt32)), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyPreferredChannelsForStereo for the device");
            CAMutex::Locker theStateLocker(mStateMutex);
            ((UInt32 *) outData)[0] = 1;
            ((UInt32 *) outData)[1] = mStreamDescription.mChannelsPerFrame > 1 ? 2 : 1;
            outDataSize = 2 * sizeof(UInt32);
            break;
        }

        case kAudioDevicePropertyPreferredChannelLayout:
            //	This property returns the default AudioChannelLayout to use for the device
            //	by default. For this device, we return a stereo ACL.
        {
            //	calcualte how big the
            UInt32 theACLSize = offsetof(AudioChannelLayout, mChannelDescriptions) + (mStreamDescription.mChannelsPerFrame * sizeof(AudioChannelDescription));
            ThrowIf(inDataSize < theACLSize, CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyPreferredChannelLayout for the device");
            ((AudioChannelLayout *) outData)->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
            ((AudioChannelLayout *) outData)->mChannelBitmap = 0;
            ((AudioChannelLayout *) outData)->mNumberChannelDescriptions = mStreamDescription.mChannelsPerFrame;
            for (theItemIndex = 0; theItemIndex < mStreamDescription.mChannelsPerFrame; ++theItemIndex) {
                if (theItemIndex <= 2) {
                    if (mStreamDescription.mChannelsPerFrame == 1) {
                        ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mChannelLabel = kAudioChannelLabel_Mono;
                    } else {
                        ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mChannelLabel = kAudioChannelLabel_Left + theItemIndex;
                    }
                } else {
                    ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mChannelLabel = kAudioChannelLabel_Unused;
                }
                ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mChannelFlags = 0;
                ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mCoordinates[0] = 0;
                ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mCoordinates[1] = 0;
                ((AudioChannelLayout *) outData)->mChannelDescriptions[theItemIndex].mCoordinates[2] = 0;
            }
            outDataSize = theACLSize;
        }
            break;

        case kAudioDevicePropertyZeroTimeStampPeriod:
            //	This property returns how many frames the HAL should expect to see between
            //	successive sample times in the zero time stamps this device provides.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_GetPropertyData: not enough space for the return value of kAudioDevicePropertyZeroTimeStampPeriod for the device");
            *reinterpret_cast<UInt32 *>(outData) = mRingBufferSize;
            outDataSize = sizeof(UInt32);
            break;

        default:
            CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            break;
    };
}

void Device::Device_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Device_GetPropertyData() method.

    switch (inAddress.mSelector) {
        case kAudioDevicePropertyNominalSampleRate:
            //	Changing the sample rate needs to be handled via the RequestConfigChange/PerformConfigChange machinery.
        {
            //	check the arguments
            ThrowIf(inDataSize != sizeof(Float64), CAException(kAudioHardwareBadPropertySizeError), "Device::Device_SetPropertyData: wrong size for the data for kAudioDevicePropertyNominalSampleRate");
            ThrowIf((*((const Float64 *) inData) != 44100.0)
                    && (*((const Float64 *) inData) != 48000.0)
                    && (*((const Float64 *) inData) != 96000.0),
                    CAException(kAudioHardwareIllegalOperationError), "Device::Device_SetPropertyData: unsupported value for kAudioDevicePropertyNominalSampleRate");

            //	we need to lock around getting the current sample rate to compare against the new rate
            UInt64 theOldSampleRate = 0;
            {
                CAMutex::Locker theStateLocker(mStateMutex);
                theOldSampleRate = (UInt64) mStreamDescription.mSampleRate;
            }

            //	make sure that the new value is different than the old value
            UInt64 theNewSampleRate = static_cast<Float64>(*reinterpret_cast<const Float64 *>(inData));
            if (theOldSampleRate != theNewSampleRate) {
                Float64 *data = new Float64(theNewSampleRate);
                //	we dispatch this so that the change can happen asynchronously
                AudioObjectID theDeviceObjectID = GetObjectID();
                CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
                    PlugIn::Host_RequestDeviceConfigurationChange(theDeviceObjectID, kHub_SampleRateChange, data);
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

bool Device::Stream_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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

bool Device::Stream_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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

UInt32 Device::Stream_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const {
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
            theAnswer = (UInt32) (mStreamDescriptions.size() * sizeof(AudioStreamRangedDescription));
            break;

        default:
            theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
            break;
    };
    return theAnswer;
}

void Device::Stream_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const {
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required.
    //	Also, since most of the data that will get returned is static, there are few instances where
    //	it is necessary to lock the state mutex.

    UInt32 theNumberItemsToFetch;
    switch (inAddress.mSelector) {
        case kAudioObjectPropertyBaseClass:
            //	The base class for kAudioStreamClassID is kAudioObjectClassID
            ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioObjectPropertyBaseClass for the volume control");
            *reinterpret_cast<AudioClassID *>(outData) = kAudioObjectClassID;
            outDataSize = sizeof(AudioClassID);
            break;

        case kAudioObjectPropertyClass:
            //	Streams are of the class, kAudioStreamClassID
            ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioObjectPropertyClass for the volume control");
            *reinterpret_cast<AudioClassID *>(outData) = kAudioStreamClassID;
            outDataSize = sizeof(AudioClassID);
            break;

        case kAudioObjectPropertyOwner:
            //	The stream's owner is the device object
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioObjectPropertyOwner for the volume control");
            *reinterpret_cast<AudioObjectID *>(outData) = GetObjectID();
            outDataSize = sizeof(AudioObjectID);
            break;

        case kAudioStreamPropertyIsActive:
            //	This property tells the device whether or not the given stream is going to
            //	be used for IO. Note that we need to take the state lock to examine this
            //	value.
        {
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyIsActive for the stream");

            //	lock the state mutex
            CAMutex::Locker theStateLocker(mStateMutex);

            //	return the requested value
            *reinterpret_cast<UInt32 *>(outData) = (UInt32) ((inAddress.mScope == kAudioObjectPropertyScopeInput) ? mInputStreamIsActive : mOutputStreamIsActive);
            outDataSize = sizeof(UInt32);
        }
            break;

        case kAudioStreamPropertyDirection:
            //	This returns whether the stream is an input stream or an output stream.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyDirection for the stream");
            *reinterpret_cast<UInt32 *>(outData) = (inObjectID == mInputStreamObjectID) ? 1 : 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyTerminalType:
            //	This returns a value that indicates what is at the other end of the stream
            //	such as a speaker or headphones, or a microphone. Values for this property
            //	are defined in <CoreAudio/AudioHardwareBase.h>
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyTerminalType for the stream");
            *reinterpret_cast<UInt32 *>(outData) = (inObjectID == mInputStreamObjectID) ? kAudioStreamTerminalTypeLine : kAudioStreamTerminalTypeLine;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyStartingChannel:
            //	This property returns the absolute channel number for the first channel in
            //	the stream. For exmaple, if a device has two output streams with two
            //	channels each, then the starting channel number for the first stream is 1
            //	and ths starting channel number fo the second stream is 3.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyStartingChannel for the stream");
            *reinterpret_cast<UInt32 *>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyLatency:
            //	This property returns any additonal presentation latency the stream has.
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyStartingChannel for the stream");
            *reinterpret_cast<UInt32 *>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat:
            //	This returns the current format of the stream in an AudioStreamBasicDescription.
            //	For devices that don't override the mix operation, the virtual format has to be the
            //	same as the physical format.
        {
            ThrowIf(inDataSize < sizeof(AudioStreamBasicDescription), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_GetPropertyData: not enough space for the return value of kAudioStreamPropertyVirtualFormat for the stream");

            //	lock the state mutex
            CAMutex::Locker theStateLocker(mStateMutex);

            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mSampleRate = static_cast<Float64>(mStreamDescription.mSampleRate);
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mFormatID = mStreamDescription.mFormatID;
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mFormatFlags = mStreamDescription.mFormatFlags;
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mBytesPerPacket = mStreamDescription.mBytesPerPacket;
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mFramesPerPacket = mStreamDescription.mFramesPerPacket;
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mBytesPerFrame = mStreamDescription.mBytesPerFrame;
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mChannelsPerFrame = mStreamDescription.mChannelsPerFrame;
            reinterpret_cast<AudioStreamBasicDescription *>(outData)->mBitsPerChannel = mStreamDescription.mBitsPerChannel;
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
            theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioStreamRangedDescription));

            //	clamp it to the number of items we have
            if (theNumberItemsToFetch > mStreamDescriptions.size()) {
                theNumberItemsToFetch = (UInt32) mStreamDescriptions.size();
            }

            for (int index = 0; index < theNumberItemsToFetch; index++) {
                AudioStreamBasicDescription description = (AudioStreamBasicDescription) mStreamDescriptions.at(index);
                ((AudioStreamRangedDescription *) outData)[index] = (AudioStreamRangedDescription) CAStreamRangedDescription(description);
            }

            //	report how much we wrote
            outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioStreamRangedDescription));
            break;
        }
        default:
            CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            break;
    };
}

void Device::Stream_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required. There is more detailed commentary about each property in the
    //	Stream_GetPropertyData() method.

    switch (inAddress.mSelector) {
        case kAudioStreamPropertyIsActive: {
            //	Changing the active state of a stream doesn't affect IO or change the structure
            //	so we can just save the state and send the notification.
            ThrowIf(inDataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_SetPropertyData: wrong size for the data for kAudioDevicePropertyNominalSampleRate");
            bool theNewIsActive = *reinterpret_cast<const UInt32 *>(inData) != 0;

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
        }
            break;

        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat: {
            //	Changing the stream format needs to be handled via the
            //	RequestConfigChange/PerformConfigChange machinery.
            ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription), CAException(kAudioHardwareBadPropertySizeError), "Device::Stream_SetPropertyData: wrong size for the data for kAudioStreamPropertyPhysicalFormat");

            const AudioStreamBasicDescription *theNewFormat = reinterpret_cast<const AudioStreamBasicDescription *>(inData);
            CAStreamBasicDescription theStreamDescription(*theNewFormat);

            ThrowIf(!theStreamDescription.IsPCM(), CAException(kAudioDeviceUnsupportedFormatError),
                    "Device::Stream_SetPropertyData: unsupported format ID for kAudioStreamPropertyPhysicalFormat");

            ThrowIf(theStreamDescription.NumberChannels() != mStreamDescription.mChannelsPerFrame, CAException(kAudioDeviceUnsupportedFormatError), "Device::Stream_SetPropertyData: unsupported channels per frame for kAudioStreamPropertyPhysicalFormat");

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
                AudioStreamBasicDescription *format = new AudioStreamBasicDescription(*theNewFormat);
                //	we dispatch this so that the change can happen asynchronously
                AudioObjectID theDeviceObjectID = GetObjectID();
                CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
                    PlugIn::Host_RequestDeviceConfigurationChange(theDeviceObjectID, kHub_StreamFormatChange, format);
                });
            }
        }
            break;

        default:
            CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
            break;
    };
}

#pragma mark Control Property Operations

bool Device::Control_HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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

bool Device::Control_IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
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

UInt32 Device::Control_GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const {
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

void Device::Control_GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const {
    //	For each object, this driver implements all the required properties plus a few extras that
    //	are useful but not required.
    //	Also, since most of the data that will get returned is static, there are few instances where
    //	it is necessary to lock the state mutex.

    Float32 theVolumeValue;
    switch (inAddress.mSelector) {
        case kAudioObjectPropertyBaseClass:
            //	The base class for kAudioVolumeControlClassID is kAudioLevelControlClassID
            ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioObjectPropertyBaseClass for the volume control");
            *reinterpret_cast<AudioClassID *>(outData) = kAudioLevelControlClassID;
            outDataSize = sizeof(AudioClassID);
            break;

        case kAudioObjectPropertyClass:
            //	Volume controls are of the class, kAudioVolumeControlClassID
            ThrowIf(inDataSize < sizeof(AudioClassID), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioObjectPropertyClass for the volume control");
            *reinterpret_cast<AudioClassID *>(outData) = kAudioVolumeControlClassID;
            outDataSize = sizeof(AudioClassID);
            break;

        case kAudioObjectPropertyOwner:
            //	The control's owner is the device object
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioObjectPropertyOwner for the volume control");
            *reinterpret_cast<AudioObjectID *>(outData) = GetObjectID();
            outDataSize = sizeof(AudioObjectID);
            break;

        case kAudioControlPropertyScope:
            //	This property returns the scope that the control is attached to.
            ThrowIf(inDataSize < sizeof(AudioObjectPropertyScope), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioControlPropertyScope for the volume control");
            *reinterpret_cast<AudioObjectPropertyScope *>(outData) = (inObjectID == mInputMasterVolumeControlObjectID) ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput;
            outDataSize = sizeof(AudioObjectPropertyScope);
            break;

        case kAudioControlPropertyElement:
            //	This property returns the element that the control is attached to.
            ThrowIf(inDataSize < sizeof(AudioObjectPropertyElement), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioControlPropertyElement for the volume control");
            *reinterpret_cast<AudioObjectPropertyElement *>(outData) = kAudioObjectPropertyElementMaster;
            outDataSize = sizeof(AudioObjectPropertyElement);
            break;

        case kAudioLevelControlPropertyScalarValue:
            //	This returns the value of the control in the normalized range of 0 to 1.
        {
            ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyScalarValue for the volume control");
            CAMutex::Locker theStateLocker(mStateMutex);
            if (inObjectID == mInputMasterVolumeControlObjectID) {
                *reinterpret_cast<Float32 *>(outData) = mMasterInputVolume;
            }
            else {
                *reinterpret_cast<Float32 *>(outData) = mMasterOutputVolume;
            }
            outDataSize = sizeof(Float32);
        }
            break;

        case kAudioLevelControlPropertyDecibelValue:
            //	This returns the dB value of the control.
        {
            ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelValue for the volume control");
            CAMutex::Locker theStateLocker(mStateMutex);
            if (inObjectID == mInputMasterVolumeControlObjectID) {
                *reinterpret_cast<Float32 *>(outData) = mVolumeCurve.ConvertScalarToDB(mMasterInputVolume);
            }
            else {
                *reinterpret_cast<Float32 *>(outData) = mVolumeCurve.ConvertScalarToDB(mMasterOutputVolume);
            }
            outDataSize = sizeof(Float32);
        }
            break;

        case kAudioLevelControlPropertyDecibelRange:
            //	This returns the dB range of the control.
            ThrowIf(inDataSize < sizeof(AudioValueRange), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelRange for the volume control");
            reinterpret_cast<AudioValueRange *>(outData)->mMinimum = mVolumeCurve.GetMinimumDB();
            reinterpret_cast<AudioValueRange *>(outData)->mMaximum = mVolumeCurve.GetMaximumDB();
            outDataSize = sizeof(AudioValueRange);
            break;

        case kAudioLevelControlPropertyConvertScalarToDecibels:
            //	This takes the scalar value in outData and converts it to dB.
            ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelValue for the volume control");

            //	clamp the value to be between 0 and 1
            theVolumeValue = *reinterpret_cast<Float32 *>(outData);
            theVolumeValue = std::min(1.0f, std::max(0.0f, theVolumeValue));

            //	do the conversion
            *reinterpret_cast<Float32 *>(outData) = mVolumeCurve.ConvertScalarToDB(theVolumeValue);

            //	report how much we wrote
            outDataSize = sizeof(Float32);
            break;

        case kAudioLevelControlPropertyConvertDecibelsToScalar:
            //	This takes the dB value in outData and converts it to scalar.
            ThrowIf(inDataSize < sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_GetPropertyData: not enough space for the return value of kAudioLevelControlPropertyDecibelValue for the volume control");

            //	clamp the value to be between kVolume_MinDB and kVolume_MaxDB
            theVolumeValue = *reinterpret_cast<Float32 *>(outData);
            theVolumeValue = std::min(kHub_Control_MaxDbVolumeValue, std::max(kHub_Control_MinDBVolumeValue, theVolumeValue));

            //	do the conversion
            *reinterpret_cast<Float32 *>(outData) = mVolumeCurve.ConvertDBToScalar(theVolumeValue);

            //	report how much we wrote
            outDataSize = sizeof(Float32);
            break;

        default:
            CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            break;
    };
}

void Device::Control_SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
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
            ThrowIf(inDataSize != sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_SetPropertyData: wrong size for the data for kAudioLevelControlPropertyScalarValue");
            theNewVolumeValue = *((const Float32 *) inData);
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
            ThrowIf(inDataSize != sizeof(Float32), CAException(kAudioHardwareBadPropertySizeError), "Device::Control_SetPropertyData: wrong size for the data for kAudioLevelControlPropertyScalarValue");
            theNewVolumeValue = *((const Float32 *) inData);
            theNewVolumeValue = std::min(kHub_Control_MaxDbVolumeValue, std::max(kHub_Control_MinDBVolumeValue, theNewVolumeValue));
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
            AudioObjectPropertyAddress theChangedProperties[] = {{kAudioLevelControlPropertyScalarValue, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}, {kAudioLevelControlPropertyDecibelValue, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}};
            PlugIn::Host_PropertiesChanged(inObjectID, 2, theChangedProperties);
        });
    }
}

#pragma mark IO Operations

void Device::ResetIO() {
    mRingBuffer.Deallocate();
    mRingBuffer.Allocate(mStreamDescription.mChannelsPerFrame, mStreamDescription.mBytesPerFrame, mRingBufferSize);

    mTicksPerFrame = CAHostTimeBase::GetFrequency() / mStreamDescription.mSampleRate;
    mAnchorHostTime = CAHostTimeBase::GetCurrentTime();
    mTimeline++;
    if (mTimeline == UINT64_MAX)
        mTimeline = 0;
}


void Device::StartIO() {
    //	Starting/Stopping IO needs to be reference counted due to the possibility of multiple clients starting IO
    CAMutex::Locker theStateLocker(mStateMutex);

    //	make sure we can start
    ThrowIf(mStartCount == UINT64_MAX, CAException(kAudioHardwareIllegalOperationError), "Device::StartIO: failed to start because the ref count was maxxed out already");

    //	we only tell the hardware to start if this is the first time IO has been started
    if (mStartCount == 0) {
        ResetIO();
    }
    ++mStartCount;
}

void Device::StopIO() {
    //	Starting/Stopping IO needs to be reference counted due to the possibility of multiple clients starting IO
    CAMutex::Locker theStateLocker(mStateMutex);

    //	we tell the hardware to stop if this is the last stop call
    if (mStartCount == 1) {
        mStartCount = 0;
        mRingBuffer.Deallocate();
    }
    else if (mStartCount > 1) {
        --mStartCount;
    }
}

void Device::GetZeroTimeStamp(Float64 &outSampleTime, UInt64 &outHostTime, UInt64 &outSeed) {
    if (mTimeline != mCurrentTimeLine) {
        mCurrentTimeLine = mTimeline;
        mNumberTimeStamps = 0;
    }

    UInt64 theCurrentHostTime;
    Float64 theHostTicksPerRingBuffer;
    Float64 theHostTickOffset;
    UInt64 theNextHostTime;

    theCurrentHostTime = CAHostTimeBase::GetCurrentTime();

    //	calculate the next host time
    theHostTicksPerRingBuffer = mTicksPerFrame * mRingBufferSize;
    theHostTickOffset = ((Float64) (mNumberTimeStamps + 1)) * theHostTicksPerRingBuffer;
    theNextHostTime = mAnchorHostTime + ((UInt64) theHostTickOffset);
    //	go to the next time if the next host time is less than the current time
    if (theNextHostTime <= theCurrentHostTime) {
        ++mNumberTimeStamps;
    }

    //	set the return values
    outSampleTime = mNumberTimeStamps * mRingBufferSize;
    outHostTime = (UInt64) (mAnchorHostTime + (mNumberTimeStamps * theHostTicksPerRingBuffer));
    outSeed = mTimeline;
}

void Device::WillDoIOOperation(UInt32 inOperationID, bool &outWillDo, bool &outWillDoInPlace) const {
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

void Device::BeginIOOperation(UInt32 /*inOperationID*/, UInt32 /*inIOBufferFrameSize*/, const AudioServerPlugInIOCycleInfo &inIOCycleInfo) {
}

void Device::DoIOOperation(AudioObjectID inStreamObjectID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo &inIOCycleInfo, void *ioMainBuffer, void * /*ioSecondaryBuffer*/) {
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

void Device::EndIOOperation(UInt32 /*inOperationID*/, UInt32 /*inIOBufferFrameSize*/, const AudioServerPlugInIOCycleInfo &inIOCycleInfo) {
}

inline void MakeBufferSilent(AudioBufferList *ioData) {
    for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
        memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
}

void Device::ReadInputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void *outBuffer) {
    AudioBuffer buffer;
    buffer.mDataByteSize = inIOBufferFrameSize * mStreamDescription.mBytesPerFrame;
    buffer.mNumberChannels = mStreamDescription.mChannelsPerFrame;
    buffer.mData = outBuffer;

    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0] = buffer;

    CARingBufferError error = mRingBuffer.Fetch(&bufferList, inIOBufferFrameSize, inSampleTime);
    if (error != kCARingBufferError_OK) {
        if (error == kCARingBufferError_CPUOverload) {
            DebugMessage("Device::ReadInputData: kCARingBufferError_CPUOverload");
        }
        else if (error == kCARingBufferError_TooMuch) {
            DebugMessage("Device::ReadInputData: kCARingBufferError_TooMuch");
        }
        else {
            DebugMessage("Device::ReadInputData: RingBufferError Unknown");
        }
        MakeBufferSilent(&bufferList);
        return;
    }

    for (int channel = 0; channel < mStreamDescription.mChannelsPerFrame; ++channel) {
        vDSP_vsmul((Float32 *) outBuffer + channel, mStreamDescription.mChannelsPerFrame, &mMasterOutputVolume,
                (Float32 *) outBuffer + channel, mStreamDescription.mChannelsPerFrame, inIOBufferFrameSize);
    }
}

void Device::WriteOutputData(UInt32 inIOBufferFrameSize, Float64 inSampleTime, void *inBuffer) {
    for (int channel = 0; channel < mStreamDescription.mChannelsPerFrame; ++channel) {
        vDSP_vsmul((Float32 *) inBuffer + channel, mStreamDescription.mChannelsPerFrame, &mMasterInputVolume,
                (Float32 *) inBuffer + channel, mStreamDescription.mChannelsPerFrame, inIOBufferFrameSize);
    }

    AudioBuffer buffer;
    buffer.mDataByteSize = inIOBufferFrameSize * mStreamDescription.mBytesPerFrame;
    buffer.mNumberChannels = mStreamDescription.mChannelsPerFrame;
    buffer.mData = inBuffer;

    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0] = buffer;

    CARingBufferError error = mRingBuffer.Store(&bufferList, inIOBufferFrameSize, inSampleTime);
    if (error != kCARingBufferError_OK) {
        if (error == kCARingBufferError_CPUOverload) {
            DebugMessage("Device::WriteOutputData: kCARingBufferError_CPUOverload");
        }
        else if (error == kCARingBufferError_TooMuch) {
            DebugMessage("Device::WriteOutputData: kCARingBufferError_TooMuch");
        }
        else {
            DebugMessage("Device::WriteOutputData: RingBufferError Unknown");
        }
    }
}

#pragma mark Implementation

void Device::PerformConfigChange(UInt64 inChangeAction, void *inChangeInfo) {
    if (inChangeAction == kHub_StreamFormatChange) {
        AudioStreamBasicDescription *theNewFormat = reinterpret_cast<AudioStreamBasicDescription *>(inChangeInfo);
        ThrowIfNULL(theNewFormat, CAException(kAudioHardwareIllegalOperationError), "Device::PerformConfigChange: illegal data for kHub_DeviceConfigurationChange");

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
    else if (inChangeAction == kHub_SampleRateChange) {
        Float64 *theNewSampleRate = reinterpret_cast<Float64 *>(inChangeInfo);
        ThrowIfNULL(theNewSampleRate, CAException(kAudioHardwareIllegalOperationError), "Device::PerformConfigChange: illegal data for kHub_DeviceConfigurationChange");

        // we need to be holding the IO and State lock to do this
        CAMutex::Locker theStateLocker(mStateMutex);
        CAMutex::Locker theIOLocker(mIOMutex);

        mStreamDescription.mSampleRate = *theNewSampleRate;

        delete theNewSampleRate;
    }
}

void Device::AbortConfigChange(UInt64 /*inChangeAction*/, void * /*inChangeInfo*/) {
    // we need to be holding the IO and State lock to do this
    CAMutex::Locker theStateLocker(mStateMutex);
    CAMutex::Locker theIOLocker(mIOMutex);
    ResetIO();
}