#include "DriverManager.h"

bool DMLoadDriver(const char* DriverPath)
{
    return false;
}

bool DMUnloadDriver(const char* DriverPath)
{
    return false;
}

//DMDriverStatus DMQueryDriverStatus(const char* DriverBundleId, DMDriverInformation* DriverInformation)
int32_t DMQueryDriverStatus(const char* DriverBundleId, DMDriverInformation* DriverInformation)
{
    return UNLOADED;
}

bool DMRegisterStatusCallback(DMStatusCallback StatusCallback)
{
    return false;
}

bool DMUnregisterStatusCallback(DMStatusCallback StatusCallback)
{
    return false;
}

