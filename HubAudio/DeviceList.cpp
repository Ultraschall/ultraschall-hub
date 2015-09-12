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

#include "DeviceList.h"
#include "CADispatchQueue.h"
#include "CACFArray.h"
#include "CACFNumber.h"
#include "CAException.h"
#include "PlugIn.h"

DeviceList::DeviceList()
    : mDeviceListMutex(new CAMutex("Hub Device List")) {
    
}

DeviceList::~DeviceList() {
    delete mDeviceListMutex;
    mDeviceListMutex = nullptr;
}

void DeviceList::AddDevice(Device *inDevice) {
    CAMutex::Locker theLocker(mDeviceListMutex);

    if (inDevice != NULL) {
        //	add it to the object map
        CAObjectMap::MapObject(inDevice->GetObjectID(), inDevice);

        //  Initialize an DeviceInfo to describe the new device
        DeviceInfo theDeviceInfo(inDevice->GetObjectID(), inDevice->getDeviceUID());

        //  put the device info in the list
        mDeviceInfoList.push_back(theDeviceInfo);

        //	activate the device
        inDevice->Activate();
    }
}

void DeviceList::RemoveDevice(Device *inDevice) {
    CAMutex::Locker theLocker(mDeviceListMutex);
    //  find it in the device list and grab an iterator for it
    if (inDevice != NULL) {
        bool wasFound = false;
        DeviceInfoList::iterator theDeviceIterator = mDeviceInfoList.begin();
        while (!wasFound && (theDeviceIterator != mDeviceInfoList.end())) {
            if (inDevice->GetObjectID() == theDeviceIterator->mDeviceObjectID) {
                wasFound = true;
                //	deactivate the device
                inDevice->Deactivate();

                //  remove the device from the list
                theDeviceIterator->mDeviceObjectID = 0;
                mDeviceInfoList.erase(theDeviceIterator);

                //	and release it
                CAObjectMap::ReleaseObject(inDevice);
            }
            else {
                ++theDeviceIterator;
            }
        }
    }
}

void DeviceList::RemoveAllDevices() {
    CAMutex::Locker theLocker(mDeviceListMutex);
    //	spin through the device list
    for (DeviceInfoList::iterator theDeviceIterator = mDeviceInfoList.begin(); theDeviceIterator != mDeviceInfoList.end(); ++theDeviceIterator) {
        //	remove the object from the list
        AudioObjectID theDeadDeviceObjectID = theDeviceIterator->mDeviceObjectID;
        theDeviceIterator->mDeviceObjectID = 0;

        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            CATry;
                //	resolve the device ID to an object
                CAObjectReleaser<Device> theDeadDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(theDeadDeviceObjectID));
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

AudioObjectID DeviceList::GetDeviceObjectID(UInt32 index) {
    CAMutex::Locker theLocker(mDeviceListMutex);
    if (index > mDeviceInfoList.size())
        return kAudioObjectUnknown;
    return mDeviceInfoList[index].mDeviceObjectID;
}

AudioObjectID DeviceList::GetDeviceObjectIDByUUID(CFStringRef uuid) {
    CAMutex::Locker theLocker(mDeviceListMutex);
    for (UInt32 theDeviceIndex = 0; theDeviceIndex < mDeviceInfoList.size(); ++theDeviceIndex) {
        if (CFStringCompare(uuid, mDeviceInfoList[theDeviceIndex].mDeviceUUID, 0) == kCFCompareEqualTo) {
            return mDeviceInfoList[theDeviceIndex].mDeviceObjectID;
        }
    }
    return kAudioObjectUnknown;
}

CFPropertyListRef DeviceList::GetSettings() const {
    CACFDictionary settings;
    CACFArray settingsDevices;
    
    for(const DeviceInfo &deviceInfo : mDeviceInfoList) {
        CAObjectReleaser<Device> theDevice(CAObjectMap::CopyObjectOfClassByObjectID<Device>(deviceInfo.mDeviceObjectID));
        ThrowIf(!theDevice.IsValid(), CAException(kAudioHardwareBadObjectError), "GetSettings: unknown device");
        CACFDictionary deviceSettings;
        deviceSettings.AddCFType(kHubSettingsKeyDeviceName, theDevice->GetDeviceName());
        deviceSettings.AddUInt32(kHubSettingsKeyDeviceChannels, theDevice->GetChannels());
        settingsDevices.AppendDictionary(deviceSettings.CopyCFDictionary());
    }
    
    settings.AddArray(kHubSettingsKeyDevices, settingsDevices.CopyCFArray());
    return settings.CopyCFMutableDictionary();
}

bool DeviceList::SetSettings(CFPropertyListRef propertyList) {
    if (CFGetTypeID(propertyList) != CFDictionaryGetTypeID())
        return false;
    
    CACFDictionary settings((CFDictionaryRef)propertyList, true);
    CACFArray settingsDevices;
    settings.GetCACFArray(kHubSettingsKeyDevices, settingsDevices);
    if (!settingsDevices.IsValid())
        return false;
    
    RemoveAllDevices();
    for (int i = 0; i < settingsDevices.GetNumberItems(); ++i) {
        CACFDictionary device;
        settingsDevices.GetCACFDictionary(i, device);
        if (device.IsValid()) {
            AddDevice(device.AsPropertyList());
        }
    }
    
    return true;
}

bool DeviceList::AddDevice(CFPropertyListRef config) {
    if (CFGetTypeID(config) != CFDictionaryGetTypeID())
        return false;
    
    CACFDictionary device((CFDictionaryRef)config, true);
    CACFString deviceUUID;
    device.GetCACFString(kHubSettingsKeyDeviceName, deviceUUID);
    if (deviceUUID.IsValid()) {
        CACFString deviceName;
        device.GetCACFString(kHubSettingsKeyDeviceName, deviceName);
        if (deviceName.IsValid()) {
            UInt32 deviceChannels = 0;
            device.GetUInt32(kHubSettingsKeyDeviceChannels, deviceChannels);
            if (deviceChannels > 0 && deviceChannels < kHubMaximumDeviceChannels) {
                auto theDevice = new Device(CAObjectMap::GetNextObjectID(), (SInt16)deviceChannels);
                theDevice->setDeviceName(deviceName.CopyCFString());
                theDevice->setDeviceUID(deviceUUID.CopyCFString());
                AddDevice(theDevice);
                return true;
            }
        }
    }
    return false;
}
