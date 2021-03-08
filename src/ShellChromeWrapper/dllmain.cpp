// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include "ShellLockScreenAPITypes.h"

extern "C"
{
    /// <summary>
    ///     Returns a snapshot of all the information displayed in the lock screen.
    /// </summary>
    /// <param name="pSnapshot">
    ///    [in/out] Pointer to a preallocated buffer to receive the snapshot.
    ///             The caller must set the "size" member to the size of the struct. Otherwise
    ///             the API will return an error.
    /// </param>
    __declspec(dllexport) HRESULT Shell_LockScreen_GetNotificationsSnapshot(
        _Inout_ LPDEVICE_LOCK_SCREEN_SNAPSHOT pSnapshot)
    {
        if (pSnapshot == NULL || pSnapshot->size < sizeof(DEVICE_LOCK_SCREEN_SNAPSHOT))
        {
            return STATUS_INVALID_PARAMETER;
        }

        DWORD size = pSnapshot->size;
        RtlZeroMemory(pSnapshot, sizeof(DEVICE_LOCK_SCREEN_SNAPSHOT));
        pSnapshot->size = size;

        /*pSnapshot->badgeCount = 0;
        pSnapshot->detailedTextCount = 3;
        pSnapshot->detailedTexts[0].isBoldText = TRUE;
        wcsncpy_s(pSnapshot->detailedTexts[0].detailedText, L"Meeting with Meow Meow", 23);
        wcsncpy_s(pSnapshot->detailedTexts[1].detailedText, L"84/Meow", 8);
        wcsncpy_s(pSnapshot->detailedTexts[2].detailedText, L"4:00 PM-4:30 PM", 16);*/

        return ERROR_SUCCESS;
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

