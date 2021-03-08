// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0

typedef BOOL (WINAPI* pCancelIoEx)(
    _In_     HANDLE       hFile,
    _In_opt_ LPOVERLAPPED lpOverlapped
);

typedef BOOL(WINAPI* pDeviceIoControl)(
    HANDLE       hDevice,
    DWORD        dwIoControlCode,
    LPVOID       lpInBuffer,
    DWORD        nInBufferSize,
    LPVOID       lpOutBuffer,
    DWORD        nOutBufferSize,
    LPDWORD      lpBytesReturned,
    LPOVERLAPPED lpOverlapped
);

typedef BOOL (WINAPI* pGetOverlappedResult)(
    HANDLE       hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD      lpNumberOfBytesTransferred,
    BOOL         bWait
);

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

typedef struct _FRAME_BUFFER_ADJUSTED
{
    UINT16 PixelWidth;
    UINT16 PixelHeight;
    UINT16 Stride;
    PVOID  FrameBufferAddress;
    PVOID  Padding;
} FRAME_BUFFER_ADJUSTED, * PFRAME_BUFFER_ADJUSTED;

typedef struct _SCREEN_SAVER_SETTINGS_ADJUSTED
{
    BOOLEAN      Enabled;
    UINT16       ActiveAreaStart;
    UINT16       ActiveAreaEnd;
    BOOLEAN      IsIlluminanceValid;
    INT32        IlluminanceInLuxes;
    UINT32       BrightnessPercentage;
    FRAME_BUFFER_ADJUSTED FrameBuffer;
} SCREEN_SAVER_SETTINGS_ADJUSTED, * PSCREEN_SAVER_SETTINGS_ADJUSTED;

#define IOCTL_OEMPANEL_SCREEN_SAVER_STATE   CTL_CODE(0x8321, 0x800 + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OEMPANEL_FRAME_BUFFER_REQUEST CTL_CODE(0x8321, 0x800 + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OEMPANEL_FRAME_BUFFER_UPDATE  CTL_CODE(0x8321, 0x800 + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern "C" {
    __declspec(dllexport) 
        BOOL
        WINAPI
        CancelIoEx(
            _In_ HANDLE hFile,
            _In_opt_ LPOVERLAPPED lpOverlapped
        )
    {
        HMODULE Dll = LoadLibrary(L"Kernel32.dll");
        if (Dll == NULL)
        {
            return FALSE;
        }

        pCancelIoEx PCancelIoEx = (pCancelIoEx)GetProcAddress(Dll, "CancelIoEx");
        BOOL result = PCancelIoEx(hFile, lpOverlapped);

        FreeLibrary(Dll);

        return result;
    }

    __declspec(dllexport) 
        BOOL
        WINAPI
        DeviceIoControl(
            _In_ HANDLE hDevice,
            _In_ DWORD dwIoControlCode,
            _In_reads_bytes_opt_(nInBufferSize) LPVOID lpInBuffer,
            _In_ DWORD nInBufferSize,
            _Out_writes_bytes_to_opt_(nOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
            _In_ DWORD nOutBufferSize,
            _Out_opt_ LPDWORD lpBytesReturned,
            _Inout_opt_ LPOVERLAPPED lpOverlapped
        )
    {
        HMODULE Dll = LoadLibrary(L"Kernel32.dll");
        if (Dll == NULL)
        {
            if (lpBytesReturned != NULL)
                *lpBytesReturned = 0;
            return FALSE;
        }

        pDeviceIoControl PDeviceIoControl = (pDeviceIoControl)GetProcAddress(Dll, "DeviceIoControl");
        BOOL result = FALSE;

        SCREEN_SAVER_SETTINGS_ADJUSTED adjustedBuffer1 = { 0 };
        FRAME_BUFFER_ADJUSTED adjustedBuffer2 = { 0 };
        FRAME_BUFFER_ADJUSTED adjustedBuffer3 = { 0 };
        BOOL Access = FALSE;
        RtlZeroMemory(&adjustedBuffer1, sizeof(SCREEN_SAVER_SETTINGS_ADJUSTED));
        RtlZeroMemory(&adjustedBuffer2, sizeof(FRAME_BUFFER_ADJUSTED));
        RtlZeroMemory(&adjustedBuffer3, sizeof(FRAME_BUFFER_ADJUSTED));

        switch (dwIoControlCode)
        {
        case IOCTL_OEMPANEL_SCREEN_SAVER_STATE:
            if (lpInBuffer != NULL && nInBufferSize == sizeof(SCREEN_SAVER_SETTINGS))
            {
                RtlCopyMemory(&adjustedBuffer1, lpInBuffer, sizeof(SCREEN_SAVER_SETTINGS));
                result = PDeviceIoControl(hDevice, dwIoControlCode, &adjustedBuffer1, sizeof(SCREEN_SAVER_SETTINGS_ADJUSTED), lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
            }
            else
            {
                result = PDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
            }
            break;
        case IOCTL_OEMPANEL_FRAME_BUFFER_REQUEST:
            if (lpInBuffer != NULL && nInBufferSize == sizeof(BOOL) && lpOutBuffer != NULL && nOutBufferSize == sizeof(FRAME_BUFFER))
            {
                RtlCopyMemory(&adjustedBuffer2, lpOutBuffer, sizeof(FRAME_BUFFER));
                Access = *(BOOL*)lpInBuffer;

                if (Access)
                {
                    nOutBufferSize += 4;
                }

                result = PDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, &adjustedBuffer2, nOutBufferSize, lpBytesReturned, lpOverlapped);
                RtlCopyMemory(lpOutBuffer, &adjustedBuffer2, sizeof(FRAME_BUFFER));
            }
            else
            {
                result = PDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
            }
            break;
        case IOCTL_OEMPANEL_FRAME_BUFFER_UPDATE:
            if (lpInBuffer != NULL && nInBufferSize == sizeof(FRAME_BUFFER))
            {
                RtlCopyMemory(&adjustedBuffer3, lpInBuffer, sizeof(FRAME_BUFFER));
                result = PDeviceIoControl(hDevice, dwIoControlCode, &adjustedBuffer3, sizeof(FRAME_BUFFER_ADJUSTED), lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
            }
            else
            {
                result = PDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
            }
            break;
        default:
            result = PDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
        }

        FreeLibrary(Dll);

        return result;
    }

    __declspec(dllexport) 
        BOOL
        WINAPI
        GetOverlappedResult(
            _In_ HANDLE hFile,
            _In_ LPOVERLAPPED lpOverlapped,
            _Out_ LPDWORD lpNumberOfBytesTransferred,
            _In_ BOOL bWait
        )
    {
        HMODULE Dll = LoadLibrary(L"Kernel32.dll");
        if (Dll == NULL)
        {
            if (lpNumberOfBytesTransferred != NULL)
                *lpNumberOfBytesTransferred = 0;
            return FALSE;
        }

        pGetOverlappedResult PGetOverlappedResult = (pGetOverlappedResult)GetProcAddress(Dll, "GetOverlappedResult");
        BOOL result = PGetOverlappedResult(hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait);

        FreeLibrary(Dll);

        return result;
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

