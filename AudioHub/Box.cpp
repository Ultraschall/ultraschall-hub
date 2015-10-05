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

#include "Box.h"
#include "AudioHubTypes.h"
#include "PlugIn.h"
#include "CAException.h"
#include "CADebugMacros.h"
#include "AudioHubTypes.h"
#include "CACFDictionary.h"

#pragma mark Construction/Destruction

Box::Box(AudioObjectID inObjectID)
        : CAObject(inObjectID, kAudioBoxClassID, kAudioObjectClassID, kAudioObjectPlugInObject),
          mBoxName(kAudioHubBoxName) {

}

void Box::Activate() {
    mAcquired = 1;
    CAObject::Activate();
}

void Box::Deactivate() {
    CAObject::Deactivate();
}

Box::~Box() {
    RemoveAllDevices();
}

bool Box::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
    bool theAnswer;
    switch (inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
        case kAudioObjectPropertyModelName:
        case kAudioObjectPropertyManufacturer:
        case kAudioObjectPropertyIdentify:
        case kAudioObjectPropertySerialNumber:
        case kAudioObjectPropertyFirmwareVersion:
            
        case kAudioObjectPropertyCustomPropertyInfoList:
        case kAudioHubCustomPropertySettings:
        case kAudioHubCustomPropertyActive:

        case kAudioBoxPropertyBoxUID:
        case kAudioBoxPropertyTransportType:
        case kAudioBoxPropertyHasAudio:
        case kAudioBoxPropertyHasVideo:
        case kAudioBoxPropertyHasMIDI:
        case kAudioBoxPropertyIsProtected:
        case kAudioBoxPropertyAcquired:
        case kAudioBoxPropertyAcquisitionFailed:
        case kAudioBoxPropertyDeviceList:
            theAnswer = true;
            break;
        default:
            theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };
    return theAnswer;
}

bool Box::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const {
    bool theAnswer;
    switch (inAddress.mSelector)
    {
        case kAudioObjectPropertyModelName:
        case kAudioObjectPropertyManufacturer:
        case kAudioObjectPropertyIdentify:
        case kAudioObjectPropertySerialNumber:
        case kAudioObjectPropertyFirmwareVersion:
            
        case kAudioBoxPropertyBoxUID:
        case kAudioBoxPropertyTransportType:
        case kAudioBoxPropertyHasAudio:
        case kAudioBoxPropertyHasVideo:
        case kAudioBoxPropertyHasMIDI:
        case kAudioBoxPropertyIsProtected:
        case kAudioBoxPropertyAcquisitionFailed:
        case kAudioBoxPropertyDeviceList:
            theAnswer = false;
            break;
            
        case kAudioHubCustomPropertySettings:
        case kAudioHubCustomPropertyActive:
        case kAudioObjectPropertyName:
            theAnswer = true;
            break;
            
        default:
            theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };
    return theAnswer;
}

UInt32 Box::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const {
    UInt32 theAnswer = 0;
    switch (inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyModelName:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyManufacturer:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyIdentify:
            theAnswer = sizeof(UInt32);
            break;
            
        case kAudioObjectPropertySerialNumber:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyFirmwareVersion:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyCustomPropertyInfoList:
            theAnswer = (UInt32)(kAudioHubCustomProperties * sizeof(AudioServerPlugInCustomPropertyInfo));
            break;

        case kAudioHubCustomPropertySettings:
            theAnswer = sizeof(CFPropertyListRef);
            break;
            
        case kAudioHubCustomPropertyActive:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioBoxPropertyBoxUID:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioBoxPropertyTransportType:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioBoxPropertyHasAudio:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioBoxPropertyHasVideo:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioBoxPropertyHasMIDI:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioBoxPropertyIsProtected:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioBoxPropertyAcquired:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioBoxPropertyAcquisitionFailed:
            theAnswer = sizeof(OSStatus);
            break;

        case kAudioBoxPropertyDeviceList: {
                if (mAcquired) {
                    CAMutex::Locker theLocker(mDeviceListMutex);
                    theAnswer = static_cast<UInt32>(mDeviceInfoList.size() * sizeof(AudioObjectID));
                } else {
                    theAnswer = 0;
                }
            }
            break;

        default:
            theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
            break;
    }
    return theAnswer;
}

void Box::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const {
    switch (inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioObjectPropertyName for the box");
            *reinterpret_cast<CFStringRef *>(outData) = mBoxName.CopyCFString();
            outDataSize = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyModelName:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioObjectPropertyModelName for the box");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubBoxModelName;
            if(*((CFStringRef*)outData) != NULL)
            {
                CFRetain(*((CFStringRef*)outData));
            }
            outDataSize = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyManufacturer:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer for the box");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubManufacturer;
            if(*((CFStringRef*)outData) != NULL)
            {
                CFRetain(*((CFStringRef*)outData));
            }
            outDataSize = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyIdentify:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioObjectPropertyIdentify for the box");
            *reinterpret_cast<UInt32 *>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;
            
        case kAudioObjectPropertySerialNumber:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioObjectPropertySerialNumber for the box");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubBoxSerialNumber;
            if(*((CFStringRef*)outData) != NULL)
            {
                CFRetain(*((CFStringRef*)outData));
            }
            outDataSize = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyFirmwareVersion:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioObjectPropertyFirmwareVersion for the box");
            *reinterpret_cast<CFStringRef *>(outData) = kAudioHubBoxFirmwareVersion;
            if(*((CFStringRef*)outData) != NULL)
            {
                CFRetain(*((CFStringRef*)outData));
            }
            outDataSize = sizeof(CFStringRef);
            break;
            
        case kAudioObjectPropertyCustomPropertyInfoList: {
            UInt32 theNumberItemsToFetch = (UInt32)(inDataSize / sizeof(AudioServerPlugInCustomPropertyInfo));
            if (theNumberItemsToFetch > kAudioHubCustomProperties) {
                theNumberItemsToFetch = kAudioHubCustomProperties;
            }
            if (theNumberItemsToFetch > 0) {
                ((AudioServerPlugInCustomPropertyInfo*)outData)[0].mSelector = kAudioHubCustomPropertySettings;
                ((AudioServerPlugInCustomPropertyInfo*)outData)[0].mPropertyDataType = kAudioServerPlugInCustomPropertyDataTypeCFPropertyList;
                ((AudioServerPlugInCustomPropertyInfo*)outData)[0].mQualifierDataType = kAudioServerPlugInCustomPropertyDataTypeNone;
            }
            if (theNumberItemsToFetch > 1) {
                ((AudioServerPlugInCustomPropertyInfo*)outData)[1].mSelector = kAudioHubCustomPropertyActive;
                ((AudioServerPlugInCustomPropertyInfo*)outData)[1].mPropertyDataType = kAudioServerPlugInCustomPropertyDataTypeCFString;
                ((AudioServerPlugInCustomPropertyInfo*)outData)[1].mQualifierDataType = kAudioServerPlugInCustomPropertyDataTypeNone;
            }
            outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioServerPlugInCustomPropertyInfo));
            break;
        }
            
        case kAudioHubCustomPropertySettings: {
            ThrowIf(inDataSize < sizeof(CFPropertyListRef), CAException(kAudioHardwareBadPropertySizeError), "UltHub_GetPlugInPropertyData: not enough space for the return value of kAudioHubCustomPropertySettings for the box");
            *reinterpret_cast<CFPropertyListRef*>(outData) = this->GetSettings();
            if(*((CFPropertyListRef*)outData) != NULL)
            {
                CFRetain(*((CFPropertyListRef*)outData));
            }
            outDataSize = sizeof(CFPropertyListRef);
            break;
        }
            
        case kAudioHubCustomPropertyActive: {
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "UltHub_GetPlugInPropertyData: not enough space for the return value of kAudioHubCustomPropertyActive for the box");
            if (mAcquired) {
                *reinterpret_cast<CFStringRef*>(outData) = CFSTR("YES");
                if(*((CFStringRef*)outData) != NULL)
                {
                    CFRetain(*((CFStringRef*)outData));
                }
            } else {
                *reinterpret_cast<CFStringRef*>(outData) = CFSTR("NO");
                if(*((CFStringRef*)outData) != NULL)
                {
                    CFRetain(*((CFStringRef*)outData));
                }

            }
            outDataSize = sizeof(CFStringRef);
            break;
        }
            
        case kAudioBoxPropertyBoxUID:
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyBoxUID for the box");
            *reinterpret_cast<CFStringRef *>(outData) = mBoxName.CopyCFString();
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioBoxPropertyTransportType:
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyTransportType for the box");
            *reinterpret_cast<UInt32 *>(outData) = kAudioDeviceTransportTypeVirtual;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioBoxPropertyHasAudio:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyHasAudio for the box");
            *reinterpret_cast<UInt32 *>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioBoxPropertyHasVideo:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyHasVideo for the box");
            *reinterpret_cast<UInt32 *>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioBoxPropertyHasMIDI:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyHasMIDI for the box");
            *reinterpret_cast<UInt32 *>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioBoxPropertyIsProtected:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyIsProtected for the box");
            *reinterpret_cast<UInt32 *>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioBoxPropertyAcquired:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyAcquired for the box");
            *reinterpret_cast<UInt32 *>(outData) = mAcquired;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioBoxPropertyAcquisitionFailed:
            ThrowIf(inDataSize < sizeof(OSStatus), CAException(kAudioHardwareBadPropertySizeError), "Box::GetPropertyData: not enough space for the return value of kAudioBoxPropertyAcquisitionFailed for the box");
            *reinterpret_cast<OSStatus *>(outData) = mAcquisitionFailed;
            outDataSize = sizeof(OSStatus);
            break;
            
        case kAudioBoxPropertyDeviceList: {
                if (mAcquired) {
                    CAMutex::Locker theLocker(mDeviceListMutex);
                    UInt32 theNumberItemsToFetch = static_cast<UInt32>(std::min(inDataSize / sizeof(AudioObjectID), mDeviceInfoList.size()));
                    AudioObjectID *theReturnedDeviceList = reinterpret_cast<AudioObjectID *>(outData);
                    for (UInt32 theDeviceIndex = 0; theDeviceIndex < theNumberItemsToFetch; ++theDeviceIndex) {
                        theReturnedDeviceList[theDeviceIndex] = mDeviceInfoList[theDeviceIndex].mDeviceObjectID;
                    }
                    outDataSize = (UInt32) (theNumberItemsToFetch * sizeof(AudioObjectID));
                } else {
                    outDataSize = 0;
                }
            }
            break;

        default:
            CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            break;
    }
}

void Box::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData) {
    switch (inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
        {
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::SetPropertyData: not enough space for the return value of kAudioObjectPropertyName for the box");
            CFStringRef* theNewName = (CFStringRef*)inData;
            if((theNewName != NULL) && (*theNewName != NULL))
            {
                CFRetain(*((CFStringRef*)theNewName));
                mBoxName = *theNewName;
            }
        }
            break;
            
        case kAudioHubCustomPropertySettings:
        {
            ThrowIf(inDataSize < sizeof(CFPropertyListRef), CAException(kAudioHardwareBadPropertySizeError), "Box::SetPropertyData: not enough space for the return value of kAudioHubCustomPropertySettings for the box");
            CFPropertyListRef* settings = (CFPropertyListRef*)inData;
            if((settings != NULL) && (*settings != NULL))
            {
                CFRetain(*((CFPropertyListRef*)settings));
                if (SetSettings(*settings)) {
                    PlugIn::GetInstance().StoreSettings();
                    if (mAcquired) {
                        AudioObjectPropertyAddress theChangedProperties[] = {
                            {kAudioPlugInPropertyDeviceList, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}
                        };
                        PlugIn::Host_PropertiesChanged(GetObjectID(), 1, theChangedProperties);
                        PlugIn::Host_PropertiesChanged(kAudioObjectPlugInObject, 1, theChangedProperties);
                    }
                }
                //CFRelease(*((CFPropertyListRef*)settings));
            }
        }
            break;
            
        case kAudioHubCustomPropertyActive:
        {
            ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "Box::SetPropertyData: not enough space for the return value of kAudioHubCustomPropertySettings for the box");
            CFStringRef* state = (CFStringRef*)inData;
            if((state != NULL) && (*state != NULL))
            {
                if (CFStringCompare(*state, CFSTR("YES"), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                    mAcquired = 1;
                if (CFStringCompare(*state, CFSTR("NO"), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                    mAcquired = 0;
                    {
                        AudioObjectPropertyAddress theChangedProperties[] = {
                            {kAudioPlugInPropertyDeviceList, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster},
                            {kAudioBoxPropertyAcquired, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}
                        };
                        PlugIn::Host_PropertiesChanged(GetObjectID(), 2, theChangedProperties);
                    }
                    {
                        AudioObjectPropertyAddress theChangedProperties[] = {
                            {kAudioPlugInPropertyDeviceList, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}
                        };
                        PlugIn::Host_PropertiesChanged(kAudioObjectPlugInObject, 1, theChangedProperties);
                    }
            }

        }
            break;
            
        default:
            CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
            break;
    }
}
