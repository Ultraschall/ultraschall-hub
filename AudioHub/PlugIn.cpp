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

#include "PlugIn.h"

#include "AudioHubTypes.h"
#include "Box.h"

#include "CAException.h"

PlugIn &PlugIn::GetInstance() {
    pthread_once(&sStaticInitializer, StaticInitializer);
    return *sInstance;
}

PlugIn::PlugIn()
        : CAObject(kAudioObjectPlugInObject, kAudioPlugInClassID, kAudioObjectClassID, 0),
          mBoxAudioObjectID(CAObjectMap::GetNextObjectID()),
          mMutex(new CAMutex("Hub Plugin")) {}

PlugIn::~PlugIn() {
    delete mMutex;
    mMutex = nullptr;
}

void PlugIn::Activate() {
    DebugMsg("PlugIn::Activate %s %d", __FILE__, __LINE__);
    CAMutex::Locker theLocker(mMutex);
    mBox = new Box(mBoxAudioObjectID);
    CAObjectMap::MapObject(mBoxAudioObjectID, mBox);
    mBox->Activate();
    CAObject::Activate();
}

void PlugIn::Deactivate() {
    CAMutex::Locker theLocker(mMutex);
    CAObject::Deactivate();
    CAObjectMap::UnmapObject(mBox->GetObjectID(), mBox);
    mBox = nullptr;
}

void PlugIn::StaticInitializer() {
    try {
        sInstance = new PlugIn;
        CAObjectMap::MapObject(kAudioObjectPlugInObject, sInstance);
        sInstance->Activate();
    }
    catch (...) {
        DebugMsg("PlugIn::StaticInitializer: failed to create the plug-in");
        delete sInstance;
        sInstance = nullptr;
    }
}

#pragma mark Property Operations

bool PlugIn::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
    bool theAnswer;
    switch (inAddress.mSelector) {
        case kAudioObjectPropertyManufacturer:
        case kAudioPlugInPropertyDeviceList:
        case kAudioPlugInPropertyTranslateUIDToDevice:
        case kAudioPlugInPropertyResourceBundle:
        case kAudioPlugInPropertyBoxList:
        case kAudioPlugInPropertyTranslateUIDToBox:
            theAnswer = true;
            break;

        default:
            theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
    };
    return theAnswer;
}

bool PlugIn::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
    bool theAnswer;
    switch (inAddress.mSelector) {
        case kAudioObjectPropertyManufacturer:
        case kAudioPlugInPropertyDeviceList:
        case kAudioPlugInPropertyTranslateUIDToDevice:
        case kAudioPlugInPropertyResourceBundle:
        case kAudioPlugInPropertyBoxList:
        case kAudioPlugInPropertyTranslateUIDToBox:
            theAnswer = false;
            break;

        default:
            theAnswer = CAObject::IsPropertySettable(inObjectID, inClientPID, inAddress);
    };
    return theAnswer;
}

UInt32 PlugIn::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const {
    UInt32 theAnswer = 0;
    switch (inAddress.mSelector) {
        case kAudioObjectPropertyManufacturer:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyOwnedObjects: {
                theAnswer = sizeof(AudioObjectID);
            }
            break;
        case kAudioPlugInPropertyDeviceList: {
            AudioObjectPropertyAddress address;
            address.mScope = kAudioObjectPropertyScopeGlobal;
            address.mSelector = kAudioBoxPropertyDeviceList;
            theAnswer = mBox->GetPropertyDataSize(inObjectID, inClientPID, address, inQualifierDataSize, inQualifierData);
            }
            break;

        case kAudioPlugInPropertyTranslateUIDToDevice:
            theAnswer = sizeof(AudioObjectID);
            break;

        case kAudioPlugInPropertyResourceBundle:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioPlugInPropertyBoxList:
            theAnswer = sizeof(AudioObjectID);
            break;
            
        case kAudioPlugInPropertyTranslateUIDToBox:
            theAnswer = sizeof(AudioObjectID);
            break;

        default:
            theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
    };
    return theAnswer;
}

void PlugIn::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const {
    switch (inAddress.mSelector) {
        case kAudioObjectPropertyManufacturer:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubManufacturer;
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyOwnedObjects: {
                *reinterpret_cast<AudioObjectID *>(outData) = mBox->GetObjectID();
                outDataSize = (UInt32) sizeof(AudioObjectID);
            }
            break;
        case kAudioPlugInPropertyDeviceList: {
            AudioObjectPropertyAddress address;
            address.mScope = kAudioObjectPropertyScopeGlobal;
            address.mSelector = kAudioBoxPropertyDeviceList;
            mBox->GetPropertyData(inObjectID, inClientPID, address, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            }
            break;

        case kAudioPlugInPropertyTranslateUIDToDevice: {
            //	This property translates the UID passed in the qualifier as a CFString into the
            //	AudioObjectID for the device the UID refers to or kAudioObjectUnknown if no device
            //	has the UID.
            ThrowIf(inQualifierDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: the qualifier size is too small for kAudioPlugInPropertyTranslateUIDToDevice");
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: not enough space for the return value of kAudioPlugInPropertyTranslateUIDToDevice");
            CAMutex::Locker theLocker(mMutex);

            AudioObjectID audioObjectID = mBox->GetDeviceObjectIDByUUID(*((CFStringRef *) inQualifierData));
            *((AudioObjectID *) outData) = audioObjectID;
            outDataSize = sizeof(AudioObjectID);
        }
            break;

        case kAudioPlugInPropertyResourceBundle:
            //	The resource bundle is a path relative to the path of the plug-in's bundle.
            //	To specify that the plug-in bundle itself should be used, we just return the
            //	empty string.
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: not enough space for the return value of kAudioPlugInPropertyResourceBundle");
            *reinterpret_cast<CFStringRef *>(outData) = CFSTR("");
            outDataSize = sizeof(CFStringRef);
            break;
            
        case kAudioPlugInPropertyBoxList: {
                //	Calculate the number of items that have been requested. Note that this
                //	number is allowed to be smaller than the actual size of the list. In such
                //	case, only that number of items will be returned
                UInt32 theNumberItemsToFetch = (UInt32) (inDataSize / sizeof(AudioObjectID));
            
                //	global scope means return all objects
                if (theNumberItemsToFetch > 1) {
                    theNumberItemsToFetch = 1;
                }

                reinterpret_cast<AudioObjectID *>(outData)[0] = mBox->GetObjectID();
                outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioObjectID));
            }
            break;
            
        case kAudioPlugInPropertyTranslateUIDToBox:
            DebugMsg("kAudioPlugInPropertyTranslateUIDToBox %s %d", __FILE__, __LINE__);
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: not enough space for the return value of kAudioPlugInPropertyTranslateUIDToBox");
            ThrowIf(inQualifierDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: the qualifier is the wrong size for kAudioPlugInPropertyTranslateUIDToBox");
            ThrowIf(inQualifierData == NULL, CAException(kAudioHardwareBadPropertySizeError), "PlugIn::GetPropertyData: no qualifier for kAudioPlugInPropertyTranslateUIDToBox");
            if(CFStringCompare(*((CFStringRef*)inQualifierData), kAudioHubBoxUID, 0) == kCFCompareEqualTo)
            {
                DebugMsg("AudioObjectID %s %d", __FILE__, __LINE__);
                *reinterpret_cast<AudioObjectID *>(outData) = mBox->GetObjectID();
            }
            else
            {
                DebugMsg("kAudioObjectUnknown %s %d", __FILE__, __LINE__);
                *((AudioObjectID*)outData) = kAudioObjectUnknown;
            }
            outDataSize = sizeof(AudioObjectID);
            break;

        default:
            CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            break;
    };
}

void PlugIn::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    switch (inAddress.mSelector) {

        default:
            CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
            break;
    };
}

#pragma mark Settings
void PlugIn::StoreSetting() {
    CFPropertyListRef settigns = mBox->GetSettings();
    sHost->WriteToStorage(sHost, kAudioHubSettingsKey, &settigns);
}

void PlugIn::RestoreSettings() {
    CFPropertyListRef settigns;
    OSStatus status = sHost->CopyFromStorage(sHost, kAudioHubSettingsKey, &settigns);
    if (settigns) {
        if (!status) {
            mBox->SetSettings(settigns);
        }
        CFRelease(settigns);
    }
}

#pragma mark Host Access
pthread_once_t PlugIn::sStaticInitializer = PTHREAD_ONCE_INIT;
PlugIn *PlugIn::sInstance = NULL;
AudioServerPlugInHostRef PlugIn::sHost = NULL;
