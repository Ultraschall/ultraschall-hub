//
//  TransportManager.hpp
//  UltraschallHubDriver
//
//  Created by Daniel Lindenfelser on 14/06/15.
//  Copyright © 2015 ultraschall.fm. All rights reserved.
//

#ifndef TransportManager_cpp
#define TransportManager_cpp

#include "CAObject.h"

class TransportManager : public CAObject {
public:
    TransportManager(AudioObjectID inObjectID);
    virtual void Activate();
    virtual void Deactivate();
    
protected:
    virtual ~TransportManager();
    
#pragma mark Property Operations
public:
    virtual bool HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;
    
    virtual bool IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const;
    
    virtual UInt32 GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const;
    
    virtual void GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const;
    
    virtual void SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);
private:
    CAMutex* mMutex;
    
    struct AudioEndPointInfo {
        AudioObjectID mAudioEndPointObjectID;
        
        AudioEndPointInfo()
        : mAudioEndPointObjectID(0)
        {
        }
        
        AudioEndPointInfo(AudioObjectID inDeviceObjectID)
        : mAudioEndPointObjectID(inDeviceObjectID)
        {
        }
    };
    
    typedef std::vector<AudioEndPointInfo> AudioEndPointList;
    AudioEndPointList mAudioEndPointList;
};

#endif /* TransportManager_cpp */
