// GlanceTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <cfgmgr32.h>
#include "BMP.h"

typedef struct _FRAME_BUFFER
{
    UINT16 PixelWidth;
    UINT16 PixelHeight;
    UINT16 Stride;
    PVOID  FrameBufferAddress;
} FRAME_BUFFER, * PFRAME_BUFFER;

typedef struct _SCREEN_SAVER_SETTINGS
{
    BOOLEAN      Enabled;
    UINT16       ActiveAreaStart;
    UINT16       ActiveAreaEnd;
    BOOLEAN      IsIlluminanceValid;
    INT32        IlluminanceInLuxes;
    UINT32       BrightnessPercentage;
    FRAME_BUFFER FrameBuffer;
} SCREEN_SAVER_SETTINGS, * PSCREEN_SAVER_SETTINGS;

#define IOCTL_OEMPANEL_SCREEN_SAVER_STATE   CTL_CODE(0x8321, 0x800 + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OEMPANEL_FRAME_BUFFER_REQUEST CTL_CODE(0x8321, 0x800 + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OEMPANEL_FRAME_BUFFER_UPDATE  CTL_CODE(0x8321, 0x800 + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define DEFINE_GUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID2(GUID_DEVINTERFACE_OEM_PANEL_DRIVER, 0xd4a3f63, 0xd08, 0x49a1, 0xb9, 0x1c, 0xc6, 0x5, 0x76, 0x89, 0x41, 0x74);

BOOL OemPanelFrameBufferUpdate(
    HANDLE DeviceInterface, 
    PFRAME_BUFFER Buffer
)
{
    DWORD bytesReturned = 0;
    return DeviceIoControl(DeviceInterface, (DWORD)IOCTL_OEMPANEL_FRAME_BUFFER_UPDATE, Buffer, sizeof(FRAME_BUFFER), NULL, 0, &bytesReturned, (LPOVERLAPPED)NULL);
}

BOOL OemPanelFrameBufferRequest(
    HANDLE DeviceInterface,
    BOOL Access, 
    PFRAME_BUFFER Buffer
)
{
    DWORD bytesReturned = 0;
    int outputBufferSize = 0;

    if (Access)
    {
        outputBufferSize = sizeof(FRAME_BUFFER);
    }

    return DeviceIoControl(DeviceInterface, (DWORD)IOCTL_OEMPANEL_FRAME_BUFFER_REQUEST, &Access, sizeof(BOOL), Buffer, outputBufferSize, &bytesReturned, (LPOVERLAPPED)NULL) && !(Access && !bytesReturned);
}

BOOL OemPanelScreenSaverState(
    HANDLE DeviceInterface,
    PSCREEN_SAVER_SETTINGS ScreenSaverSettings
)
{
    DWORD bytesReturned = 0;
    return DeviceIoControl(DeviceInterface, (DWORD)IOCTL_OEMPANEL_SCREEN_SAVER_STATE, ScreenSaverSettings, sizeof(SCREEN_SAVER_SETTINGS), NULL, 0, &bytesReturned, (LPOVERLAPPED)NULL);
}

BOOL SetPanelScreenSaver(
    HANDLE DeviceInterface, 
    LPCSTR BMPImagePath, 
    UINT16 OriginX,
    UINT16 OriginY,
    PSCREEN_SAVER_SETTINGS ScreenSaverSettings, 
    BOOLEAN Enable, 
    INT32 Lux, 
    UINT8 BrightnessPercentage
)
{
    BOOL Result = FALSE;

    UINT16 x, y;
    UINT32* buffer;

    ScreenSaverSettings->Enabled = Enable;

    if (Enable)
    {
        BMP bmp = BMP(BMPImagePath);

        Result = OemPanelFrameBufferRequest(DeviceInterface, TRUE, &ScreenSaverSettings->FrameBuffer);
        if (!Result)
        {
            wprintf(L"oops 1\n");
            goto exit;
        }

        UINT64 BufferSize = ScreenSaverSettings->FrameBuffer.PixelHeight * ScreenSaverSettings->FrameBuffer.Stride / 4;
        ScreenSaverSettings->FrameBuffer.FrameBufferAddress = new UINT32[BufferSize];

        UINT16 width = min(ScreenSaverSettings->FrameBuffer.PixelWidth, bmp.bmp_info_header.width);
        UINT16 height = min(ScreenSaverSettings->FrameBuffer.PixelHeight, bmp.bmp_info_header.height);

        if (width + OriginX > ScreenSaverSettings->FrameBuffer.PixelWidth)
        {
            OriginX = ScreenSaverSettings->FrameBuffer.PixelWidth - width;
        }

        if (height + OriginY > ScreenSaverSettings->FrameBuffer.PixelHeight)
        {
            OriginY = ScreenSaverSettings->FrameBuffer.PixelHeight - height;
        }

        ScreenSaverSettings->ActiveAreaStart = OriginY;
        ScreenSaverSettings->ActiveAreaEnd = height + OriginY - 1;

        ScreenSaverSettings->IsIlluminanceValid = TRUE;
        ScreenSaverSettings->BrightnessPercentage = BrightnessPercentage;
        ScreenSaverSettings->IlluminanceInLuxes = Lux;

        for (x = OriginX; x < (width + OriginX); x++)
        {
            for (y = OriginY; y < (height + OriginY); y++)
            {
                buffer = (UINT32*)ScreenSaverSettings->FrameBuffer.FrameBufferAddress + (y * ScreenSaverSettings->FrameBuffer.Stride / 4) + x;

                UINT64 index = (((UINT64)bmp.bmp_info_header.height - (UINT64)1 - (UINT64)y + OriginY) * (UINT64)bmp.bmp_info_header.width + (UINT64)x - OriginX) * (UINT64)3;

                if (index + (UINT64)2 > bmp.data.size())
                    continue;

                *buffer = (
                    bmp.data[index + 2] << 16 | // Blue
                    bmp.data[index + 1] << 8  | // Green
                    bmp.data[index]);           // Red
            }
        }

        Result = OemPanelScreenSaverState(DeviceInterface, ScreenSaverSettings);
        Result = OemPanelScreenSaverState(DeviceInterface, ScreenSaverSettings);
        Result = OemPanelScreenSaverState(DeviceInterface, ScreenSaverSettings);
        wprintf(L"oops 2 %d\n", GetLastError());
    }
    else
    {
        Result = OemPanelFrameBufferRequest(DeviceInterface, FALSE, NULL);
        wprintf(L"oops 3\n");
    }

exit:

    if (ScreenSaverSettings->FrameBuffer.FrameBufferAddress)
    {
        delete[] ScreenSaverSettings->FrameBuffer.FrameBufferAddress;
        ScreenSaverSettings->FrameBuffer.FrameBufferAddress = NULL;
    }

    RtlZeroMemory(&ScreenSaverSettings->FrameBuffer, sizeof(ScreenSaverSettings->FrameBuffer));

    return Result;
}

HRESULT FindDeviceName(
    const GUID* InterfaceGuid, 
    wchar_t** DeviceName
)
{
    wchar_t* interfaceList = NULL;
    ULONG interfaceListSize = 0;

    if (NULL == InterfaceGuid || NULL == DeviceName)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (CR_SUCCESS != CM_Get_Device_Interface_List_Size(&interfaceListSize, (LPGUID)InterfaceGuid, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT))
    {
        return ERROR_INVALID_PARAMETER;
    }

    interfaceList = new wchar_t[interfaceListSize];

    if (NULL == interfaceList)
    {
        return ERROR_OUTOFMEMORY;
    }

    if (CR_SUCCESS != CM_Get_Device_Interface_List((LPGUID)InterfaceGuid, NULL, (wchar_t*)interfaceList, interfaceListSize, CM_GET_DEVICE_INTERFACE_LIST_PRESENT))
    {
        delete[] interfaceList;
        return ERROR_DEVICE_UNREACHABLE;
    }

    *DeviceName = interfaceList;

    return ERROR_SUCCESS;
}

HRESULT InitializeInterface(
    const GUID* InterfaceGuid, 
    HANDLE* DeviceInterface
)
{
    wchar_t* DeviceName = NULL;

    if (NULL == InterfaceGuid || NULL == DeviceInterface)
    {
        return ERROR_INVALID_PARAMETER;
    }

    HRESULT hr = FindDeviceName(InterfaceGuid, &DeviceName);
    if (hr < 0 || DeviceName == NULL)
    {
        return ERROR_DEVICE_NOT_AVAILABLE;
    }

    *DeviceInterface = CreateFile((LPCWSTR)DeviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == *DeviceInterface)
    {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    return ERROR_SUCCESS;
}

VOID DeInitialize(HANDLE DeviceInterface)
{
    if (INVALID_HANDLE_VALUE != DeviceInterface)
    {
        CloseHandle(DeviceInterface);
    }
}

int main(int argc, char** argv)
{
    HANDLE DeviceInterface = INVALID_HANDLE_VALUE;
    SCREEN_SAVER_SETTINGS ScreenSaverSettings;
    HRESULT Status;
    int ReturnCode = 0;
    BOOL Result = TRUE;

    Status = InitializeInterface(&GUID_DEVINTERFACE_OEM_PANEL_DRIVER, &DeviceInterface);
    if (Status < 0)
    {
        wprintf(L"Error while initializing interface\n");
        ReturnCode = 1;
        goto exit;
    }

    if (argc == 2)
    {
        Result = SetPanelScreenSaver(DeviceInterface, argv[1], 0, 0, &ScreenSaverSettings, TRUE, 25, 25);
    }
    else if (argc == 4)
    {
        Result = SetPanelScreenSaver(DeviceInterface, argv[1], atoi(argv[2]), atoi(argv[3]), &ScreenSaverSettings, TRUE, 25, 25);
    }
    else
    {
        Result = SetPanelScreenSaver(DeviceInterface, NULL, 0, 0, &ScreenSaverSettings, FALSE, 0, 0);
    }

    if (!Result)
    {
        wprintf(L"Error while toggling screen saver\n");
        ReturnCode = 1;
        goto cleanup;
    }

    cleanup:
    DeInitialize(DeviceInterface);

    exit:
    return ReturnCode;
}