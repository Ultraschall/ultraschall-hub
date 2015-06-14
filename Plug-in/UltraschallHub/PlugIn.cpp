//
//  PlugIn.cpp
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 02/11/14.
//  Copyright (c) 2014 ultraschall.fm. All rights reserved.
//

#include "PlugIn.h"

#include "CAException.h"
#include "CADispatchQueue.h"
#include "CADebugMacros.h"
#include "CABitOperations.h"
#include "CACFDictionary.h"
#include "Device.h"

UltHub_PlugIn& UltHub_PlugIn::GetInstance()
{
    pthread_once(&sStaticInitializer, StaticInitializer);
    return *sInstance;
}

UltHub_PlugIn::UltHub_PlugIn()
    : CAObject(kAudioObjectPlugInObject, kAudioPlugInClassID, kAudioObjectClassID, 0)
    , mDeviceInfoList()
    , mMutex(new CAMutex("Ultraschall Plugin"))
{
    mCurrentSettings = NULL;
    mCurrentSettings = ReadSettings();
}

UltHub_PlugIn::~UltHub_PlugIn()
{
    delete mMutex;
    mMutex = nullptr;
    CFRelease(mCurrentSettings);
}

void UltHub_PlugIn::Activate()
{
    CAObject::Activate();

    InitializeDevices();
}

void UltHub_PlugIn::Deactivate()
{
    CAMutex::Locker theLocker(mMutex);
    CAObject::Deactivate();
    _RemoveAllDevices();
}

void UltHub_PlugIn::StaticInitializer()
{
    try {
        sInstance = new UltHub_PlugIn;
        CAObjectMap::MapObject(kAudioObjectPlugInObject, sInstance);
        sInstance->Activate();
    }
    catch (...) {
        DebugMsg("UltHub_PlugIn::StaticInitializer: failed to create the plug-in");
        delete sInstance;
        sInstance = nullptr;
    }
}

#pragma mark Settings
bool UltHub_PlugIn::ValidateSettings(CFPropertyListRef propertyListRef)
{
    if (CFGetTypeID(mCurrentSettings) == CFDictionaryGetTypeID()) {
        CFDictionaryRef root = (CFDictionaryRef)mCurrentSettings;
        if (CFDictionaryContainsKey(root, CFSTR("Devices"))) {
            CFArrayRef devices = (CFArrayRef)CFDictionaryGetValue(root, CFSTR("Devices"));
            if (devices != NULL) {
                for (int index = 0; index < CFArrayGetCount(devices); index++) {
                    CFDictionaryRef device = (CFDictionaryRef)CFArrayGetValueAtIndex(devices, index);
                    if (device != NULL) {
                        if (CFDictionaryContainsKey(device, CFSTR("UUID"))) {
                            if (CFDictionaryContainsKey(device, CFSTR("Name"))) {
                                if (CFDictionaryContainsKey(device, CFSTR("Channels"))) {
                                    CFStringRef uuid = (CFStringRef)CFDictionaryGetValue(device, CFSTR("UUID"));
                                    CFStringRef name = (CFStringRef)CFDictionaryGetValue(device, CFSTR("Name"));
                                    CFNumberRef channels = (CFNumberRef)CFDictionaryGetValue(device, CFSTR("Channels"));

                                    if (CFStringGetLength(uuid) <= 0)
                                        return false;

                                    if (CFStringGetLength(name) <= 0)
                                        return false;

                                    SInt16 c = 0;
                                    if (CFNumberGetValue(channels, CFNumberType::kCFNumberSInt16Type, &c)) {
                                        if (c != 1 || !IsPowerOfTwo(c)) {
                                            return false;
                                        }
                                    }

                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

CFPropertyListRef UltHub_PlugIn::ReadSettings() const
{
    CFBundleRef myBundle = CFBundleGetBundleWithIdentifier(CFSTR(kUltraschallHub_BundleID));
    CFURLRef settingsURL = CFBundleCopyResourceURL(myBundle, CFSTR("Devices"), CFSTR("plist"), NULL);

    CFDataRef resourceData;
    SInt32 errorCode;
    // TODO: move to new api
    Boolean status = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, settingsURL, &resourceData,
                                                              NULL, NULL, &errorCode);

    if (!status) {
        CFRelease(settingsURL);
        // Handle the error
        return NULL;
    }
    // Reconstitute the dictionary using the XML data
    CFErrorRef myError;
    CFPropertyListRef propertyList = CFPropertyListCreateWithData(kCFAllocatorDefault, resourceData, kCFPropertyListImmutable, NULL, &myError);

    if (propertyList != NULL) {
    }

    // Handle any errors
    CFRelease(resourceData);
    CFRelease(settingsURL);

    return propertyList;
}

bool UltHub_PlugIn::WriteSettings()
{
    CFBundleRef myBundle = CFBundleGetBundleWithIdentifier(CFSTR(kUltraschallHub_BundleID));
    CFURLRef mySettingsURL = CFBundleCopyResourceURL(myBundle, CFSTR("Devices"), CFSTR("plist"), NULL);

    CFWriteStreamRef myStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, mySettingsURL);
    CFErrorRef myError;
    CFPropertyListWrite(mCurrentSettings, myStream, kCFPropertyListXMLFormat_v1_0, NULL, &myError);

    CFRelease(mySettingsURL);
    CFRelease(myStream);

    if (myError != 0) {
        return false;
    }

    return true;
}

#pragma mark Property Operations

bool UltHub_PlugIn::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyManufacturer:
    case kAudioPlugInPropertyDeviceList:
    case kAudioPlugInPropertyTranslateUIDToDevice:
    case kAudioPlugInPropertyResourceBundle:
        theAnswer = true;
        break;

    default:
        theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
    };
    return theAnswer;
}

bool UltHub_PlugIn::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyManufacturer:
    case kAudioPlugInPropertyDeviceList:
    case kAudioPlugInPropertyTranslateUIDToDevice:
    case kAudioPlugInPropertyResourceBundle:
        theAnswer = false;
        break;

    default:
        theAnswer = CAObject::IsPropertySettable(inObjectID, inClientPID, inAddress);
    };
    return theAnswer;
}

UInt32 UltHub_PlugIn::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    UInt32 theAnswer = 0;
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyManufacturer:
        theAnswer = sizeof(CFStringRef);
        break;

    case kAudioObjectPropertyOwnedObjects:
    case kAudioPlugInPropertyDeviceList: {
        CAMutex::Locker theLocker(mMutex);
        theAnswer = static_cast<UInt32>(mDeviceInfoList.size() * sizeof(AudioObjectID));
    } break;

    case kAudioPlugInPropertyTranslateUIDToDevice:
        theAnswer = sizeof(AudioObjectID);
        break;

    case kAudioPlugInPropertyResourceBundle:
        theAnswer = sizeof(CFStringRef);
        break;

    default:
        theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
    };
    return theAnswer;
}

void UltHub_PlugIn::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
    switch (inAddress.mSelector) {
    case kAudioObjectPropertyManufacturer:
        ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "UltHub_PlugIn::GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer");
        *reinterpret_cast<CFStringRef*>(outData) = CFSTR(kUltraschallHub_Manufacturer);
        outDataSize = sizeof(CFStringRef);
        break;

    case kAudioObjectPropertyOwnedObjects:
    case kAudioPlugInPropertyDeviceList:
        //	The plug-in object only owns devices, so the the owned object list and the device
        //	list are actually the same thing. We need to be holding the mutex to access the
        //	device info list.
        {
            CAMutex::Locker theLocker(mMutex);

            //	Calculate the number of items that have been requested. Note that this
            //	number is allowed to be smaller than the actual size of the list. In such
            //	case, only that number of items will be returned
            UInt32 theNumberItemsToFetch = static_cast<UInt32>(std::min(inDataSize / sizeof(AudioObjectID), mDeviceInfoList.size()));

            //	go through the device list and copy out the devices' object IDs
            AudioObjectID* theReturnedDeviceList = reinterpret_cast<AudioObjectID*>(outData);
            for (UInt32 theDeviceIndex = 0; theDeviceIndex < theNumberItemsToFetch; ++theDeviceIndex) {
                theReturnedDeviceList[theDeviceIndex] = mDeviceInfoList[theDeviceIndex].mDeviceObjectID;
            }

            //	say how much we returned
            outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioObjectID));
        }
        break;

    case kAudioPlugInPropertyTranslateUIDToDevice:
        //	This property translates the UID passed in the qualifier as a CFString into the
        //	AudioObjectID for the device the UID refers to or kAudioObjectUnknown if no device
        //	has the UID.
        ThrowIf(inQualifierDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "UltHub_PlugIn::GetPropertyData: the qualifier size is too small for kAudioPlugInPropertyTranslateUIDToDevice");
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_PlugIn::GetPropertyData: not enough space for the return value of kAudioPlugInPropertyTranslateUIDToDevice");
        outDataSize = sizeof(AudioObjectID);
        break;

    case kAudioPlugInPropertyResourceBundle:
        //	The resource bundle is a path relative to the path of the plug-in's bundle.
        //	To specify that the plug-in bundle itself should be used, we just return the
        //	empty string.
        ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "UltHub_GetPlugInPropertyData: not enough space for the return value of kAudioPlugInPropertyResourceBundle");
        *reinterpret_cast<CFStringRef*>(outData) = CFSTR("");
        outDataSize = sizeof(CFStringRef);
        break;

    default:
        CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
        break;
    };
}

void UltHub_PlugIn::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    switch (inAddress.mSelector) {

    default:
        CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
        break;
    };
}

#pragma mark Device List Management

void UltHub_PlugIn::InitializeDevices()
{
    if (CFGetTypeID(mCurrentSettings) == CFDictionaryGetTypeID()) {
        CFDictionaryRef root = (CFDictionaryRef)mCurrentSettings;
        if (CFDictionaryContainsKey(root, CFSTR("Devices"))) {
            CFArrayRef devices = (CFArrayRef)CFDictionaryGetValue(root, CFSTR("Devices"));
            if (devices != NULL) {
                for (int index = 0; index < CFArrayGetCount(devices); index++) {
                    CFDictionaryRef device = (CFDictionaryRef)CFArrayGetValueAtIndex(devices, index);
                    if (device != NULL) {
                        if (CFDictionaryContainsKey(device, CFSTR("UUID"))) {
                            if (CFDictionaryContainsKey(device, CFSTR("Name"))) {
                                if (CFDictionaryContainsKey(device, CFSTR("Channels"))) {
                                    CFStringRef uuid = (CFStringRef)CFDictionaryGetValue(device, CFSTR("UUID"));
                                    CFStringRef name = (CFStringRef)CFDictionaryGetValue(device, CFSTR("Name"));
                                    CFNumberRef channels = (CFNumberRef)CFDictionaryGetValue(device, CFSTR("Channels"));

                                    UltHub_Device* theNewDevice = NULL;
                                    //	make the new device object
                                    AudioObjectID theNewDeviceObjectID = CAObjectMap::GetNextObjectID();
                                    SInt16 c = 0;
                                    if (CFNumberGetValue(channels, CFNumberType::kCFNumberSInt16Type, &c)) {
                                        theNewDevice = new UltHub_Device(theNewDeviceObjectID, c);
                                    }
                                    else {
                                        theNewDevice = new UltHub_Device(theNewDeviceObjectID);
                                    }
                                    theNewDevice->setDeviceUID(uuid);
                                    theNewDevice->setDeviceName(name);
                                    //	add it to the object map
                                    CAObjectMap::MapObject(theNewDeviceObjectID, theNewDevice);

                                    //	add it to the device list
                                    AddDevice(theNewDevice);

                                    //	activate the device
                                    theNewDevice->Activate();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //	this will change the owned object list and the device list
    AudioObjectPropertyAddress theChangedProperties[] = { kAudioObjectPropertyOwnedObjects,
                                                          kAudioObjectPropertyScopeGlobal,
                                                          kAudioObjectPropertyElementMaster,

                                                          kAudioObjectPropertyScopeGlobal,
                                                          kAudioObjectPropertyElementMaster,

                                                          kAudioPlugInPropertyDeviceList,
                                                          kAudioObjectPropertyScopeGlobal,
                                                          kAudioObjectPropertyElementMaster };
    Host_PropertiesChanged(GetObjectID(), 3, theChangedProperties);
}

void UltHub_PlugIn::AddDevice(UltHub_Device* inDevice)
{
    CAMutex::Locker theLocker(mMutex);
    _AddDevice(inDevice);
}

void UltHub_PlugIn::RemoveDevice(UltHub_Device* inDevice)
{
    CAMutex::Locker theLocker(mMutex);
    _RemoveDevice(inDevice);
}

void UltHub_PlugIn::_AddDevice(UltHub_Device* inDevice)
{
    if (inDevice != NULL) {
        //  Initialize an DeviceInfo to describe the new device
        DeviceInfo theDeviceInfo(inDevice->GetObjectID());

        //  put the device info in the list
        mDeviceInfoList.push_back(theDeviceInfo);
    }
}

void UltHub_PlugIn::_RemoveDevice(UltHub_Device* inDevice)
{
    //  find it in the device list and grab an iterator for it
    if (inDevice != NULL) {
        bool wasFound = false;
        DeviceInfoList::iterator theDeviceIterator = mDeviceInfoList.begin();
        while (!wasFound && (theDeviceIterator != mDeviceInfoList.end())) {
            if (inDevice->GetObjectID() == theDeviceIterator->mDeviceObjectID) {
                wasFound = true;

                //  remove the device from the list
                theDeviceIterator->mDeviceObjectID = 0;
                mDeviceInfoList.erase(theDeviceIterator);
            }
            else {
                ++theDeviceIterator;
            }
        }
    }
}

void UltHub_PlugIn::_RemoveAllDevices()
{
    //	spin through the device list
    for (DeviceInfoList::iterator theDeviceIterator = mDeviceInfoList.begin(); theDeviceIterator != mDeviceInfoList.end(); ++theDeviceIterator) {
        //	remove the object from the list
        AudioObjectID theDeadDeviceObjectID = theDeviceIterator->mDeviceObjectID;
        theDeviceIterator->mDeviceObjectID = 0;

        //	asynchronously get rid of the device since we are holding the plug-in's state lock
        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            CATry;
                //	resolve the device ID to an object
                CAObjectReleaser<UltHub_Device> theDeadDevice(CAObjectMap::CopyObjectOfClassByObjectID<UltHub_Device>(theDeadDeviceObjectID));
                if (theDeadDevice.IsValid()) {
                    //	deactivate the device
                    theDeadDevice->Deactivate();

                    //	and release it
                    CAObjectMap::ReleaseObject(theDeadDevice);
                }
            CACatch;
        });
    }
}

#pragma mark Host Access
pthread_once_t UltHub_PlugIn::sStaticInitializer = PTHREAD_ONCE_INIT;
UltHub_PlugIn* UltHub_PlugIn::sInstance = NULL;
AudioServerPlugInHostRef UltHub_PlugIn::sHost = NULL;
