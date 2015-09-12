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

#ifndef __Box__
#define __Box__

#include <CoreAudio/AudioServerPlugin.h>

#include "CAObject.h"
#include "CACFString.h"
#include "DeviceList.h"

class Box : public CAObject, public DeviceList {
#pragma mark Construction/Destruction
public:
    Box(AudioObjectID inObjectID);
    void Activate();
    void Deactivate();

protected:
    ~Box();

#pragma mark Property Operations
public:
    bool HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    bool IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress) const;
    UInt32 GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData) const;
    void GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, UInt32 &outDataSize, void *outData) const;
    void SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress &inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

private:
    CACFString mBoxName;
    UInt32 mAcquired = 0;
    OSStatus mAcquisitionFailed = 0;
};

#endif /* __Box__ */
