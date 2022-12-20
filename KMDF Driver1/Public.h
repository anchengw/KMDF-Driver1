/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_KMDFDriver1,
    0x4d07e5ae,0xce15,0x4e44,0xbd,0x48,0x54,0x1c,0x51,0x31,0x06,0x9c);
// {4d07e5ae-ce15-4e44-bd48-541c5131069c}
