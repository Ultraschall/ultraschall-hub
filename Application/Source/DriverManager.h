#ifndef __SOUNDFLOWER_TOOL_DRIVER_MANAGER_H_INCL__
#define __SOUNDFLOWER_TOOL_DRIVER_MANAGER_H_INCL__

#include <stdbool.h>
#include <inttypes.h>

bool DMLoadDriver(const char* DriverPath);
bool DMUnloadDriver(const char* DriverPath);

typedef struct
{
    uint32_t Index;
    uint32_t Refs;
    uint64_t Address;
    uint64_t Size;
    uint64_t Wired;
} DMDriverInformation;

typedef enum
{
    UNLOADED = 0,
    LOADED
} DMDriverStatus;

//DMDriverStatus DMQueryDriverStatus(const char* DriverBundleId, DMDriverInformation* DriverInformation);
int32_t DMQueryDriverStatus(const char* DriverBundleId, DMDriverInformation* DriverInformation);

typedef void (*DMStatusCallback)(const DMDriverInformation* DriverInformation);

bool DMRegisterStatusCallback(DMStatusCallback StatusCallback);
bool DMUnregisterStatusCallback(DMStatusCallback StatusCallback);

#endif // __SOUNDFLOWER_TOOL_DRIVER_MANAGER_H_INCL__


