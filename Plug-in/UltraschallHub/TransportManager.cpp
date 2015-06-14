//
//  TransportManager.cpp
//  UltraschallHubDriver
//
//  Created by Daniel Lindenfelser on 14/06/15.
//  Copyright © 2015 ultraschall.fm. All rights reserved.
//

#include "CAException.h"
#include "CADebugMacros.h"

#include "TransportManager.hpp"

TransportManager::TransportManager(AudioObjectID inObjectID)
: CAObject(inObjectID, kAudioTransportManagerClassID, kAudioObjectClassID, kAudioObjectPlugInObject) {
    
}

void TransportManager::Activate()
{
    //	call the super-class, which just marks the object as active
    CAObject::Activate();
}

void TransportManager::Deactivate()
{
    //	mark the object inactive by calling the super-class
    CAObject::Deactivate();
}

TransportManager::~TransportManager()
{
}

#pragma mark Property Operations

bool TransportManager::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer;
    switch (inAddress.mSelector) {
        case kAudioTransportManagerPropertyEndPointList:
        case kAudioTransportManagerPropertyTranslateUIDToEndPoint:
        case kAudioTransportManagerPropertyTransportType:
            theAnswer = true;
            break;
            
        default:
            theAnswer = CAObject::HasProperty(inObjectID, inClientPID, inAddress);
    };
    return theAnswer;

}

bool TransportManager::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer;
    switch (inAddress.mSelector) {
        case kAudioTransportManagerPropertyEndPointList:
        case kAudioTransportManagerPropertyTranslateUIDToEndPoint:
        case kAudioTransportManagerPropertyTransportType:
            theAnswer = false;
            break;
            
        default:
            theAnswer = CAObject::IsPropertySettable(inObjectID, inClientPID, inAddress);
    };
    return theAnswer;
}

UInt32 TransportManager::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    UInt32 theAnswer = 0;
    switch (inAddress.mSelector) {
        case kAudioTransportManagerPropertyEndPointList: {
            CAMutex::Locker theLocker(mMutex);
            theAnswer = static_cast<UInt32>(mAudioEndPointList.size() * sizeof(AudioObjectID));
        } break;
            
        case kAudioTransportManagerPropertyTranslateUIDToEndPoint:
            theAnswer = sizeof(CFStringRef);
            break;
            
        case kAudioTransportManagerPropertyTransportType:
            theAnswer = sizeof(UInt32);
            break;
            
        default:
            theAnswer = CAObject::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
    };
    return theAnswer;
}

void TransportManager::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
    switch (inAddress.mSelector) {
        case kAudioTransportManagerPropertyEndPointList: {
            CAMutex::Locker theLocker(mMutex);
            UInt32 theNumberItemsToFetch = static_cast<UInt32>(std::min(inDataSize / sizeof(AudioObjectID), mAudioEndPointList.size()));
            
            AudioObjectID* theReturnedDeviceList = reinterpret_cast<AudioObjectID*>(outData);
            for (UInt32 theDeviceIndex = 0; theDeviceIndex < theNumberItemsToFetch; ++theDeviceIndex) {
                theReturnedDeviceList[theDeviceIndex] = mAudioEndPointList[theDeviceIndex].mAudioEndPointObjectID;
            }
            
            outDataSize = (UInt32)(theNumberItemsToFetch * sizeof(AudioObjectID));
        } break;
            
        case kAudioTransportManagerPropertyTranslateUIDToEndPoint:
            ThrowIf(inQualifierDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "TransportManager::GetPropertyData: the qualifier size is too small for kAudioTransportManagerPropertyTranslateUIDToEndPoint");
            ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "TransportManager::GetPropertyData: not enough space for the return value of kAudioTransportManagerPropertyTranslateUIDToEndPoint");
            
            outDataSize = sizeof(AudioObjectID);
            break;
            
        case kAudioTransportManagerPropertyTransportType:
            ThrowIf(inDataSize < sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "TransportManager::GetPropertyData: not enough space for the return value of kAudioTransportManagerPropertyTransportType");
            *reinterpret_cast<UInt32*>(outData) = kAudioDeviceTransportTypeVirtual;
            outDataSize = sizeof(UInt32);
            break;
            
        default:
            CAObject::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
            break;
    };

}

void TransportManager::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    switch (inAddress.mSelector) {
            
        default:
            CAObject::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
            break;
    };
}
