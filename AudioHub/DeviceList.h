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

#ifndef __DeviceList__
#define __DeviceList__

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include "CAObject.h"
#include "Device.h"

class DeviceList {
public:
    DeviceList();
    ~DeviceList();
    void AddDevice(Device *inDevice);
    void RemoveDevice(Device *inDevice);
    void RemoveAllDevices();
    AudioObjectID GetDeviceObjectID(UInt32 index);
    UInt32 GetDeviceObjectIDByUUID(CFStringRef uuid);
    
    CFPropertyListRef GetSettings() const;
    bool SetSettings(CFPropertyListRef settings);
    bool AddDevice(CFPropertyListRef config);
    UInt32 NumDevices();
    
protected:
    void PropertiesChanged();
    struct DeviceInfo {
    DeviceInfo() : mDeviceObjectID(0), mDeviceUUID(CFSTR("")) {}
    DeviceInfo(AudioObjectID inDeviceObjectID, CFStringRef inDeviceUUID)
        : mDeviceObjectID(inDeviceObjectID), mDeviceUUID(inDeviceUUID) {}
        AudioObjectID mDeviceObjectID;
        CFStringRef mDeviceUUID;
    };
    typedef std::vector<DeviceInfo> DeviceInfoList;
    DeviceInfoList mDeviceInfoList;
    CAMutex *mDeviceListMutex;
};

#endif /* __DeviceList__ */
