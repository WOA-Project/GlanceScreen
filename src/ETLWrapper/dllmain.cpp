// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "dllmain2.h"
#include <stdlib.h>
#include <fstream>

typedef ULONG (WINAPI* PEventWrite)(
    REGHANDLE              RegHandle,
    PCEVENT_DESCRIPTOR     EventDescriptor,
    ULONG                  UserDataCount,
    PEVENT_DATA_DESCRIPTOR UserData
    );

typedef ULONG(WINAPI* PEventWriteTransfer)(
    REGHANDLE              RegHandle,
    PCEVENT_DESCRIPTOR     EventDescriptor,
    LPCGUID                ActivityId,
    LPCGUID                RelatedActivityId,
    ULONG                  UserDataCount,
    PEVENT_DATA_DESCRIPTOR UserData
);

typedef ULONG(WINAPI* PEventSetInformation)(
    REGHANDLE        RegHandle,
    EVENT_INFO_CLASS InformationClass,
    PVOID            EventInformation,
    ULONG            InformationLength
    );

typedef ULONG(WINAPI* PEventRegister)(
    LPCGUID         ProviderId,
    PENABLECALLBACK EnableCallback,
    PVOID           CallbackContext,
    PREGHANDLE      RegHandle
    );

typedef ULONG(WINAPI* PEventUnregister)(
    REGHANDLE RegHandle
);

typedef ULONG(WINAPI* PEventRegister)(
    LPCGUID         ProviderId,
    PENABLECALLBACK EnableCallback,
    PVOID           CallbackContext,
    PREGHANDLE      RegHandle
    );

extern "C" {
    __declspec(dllexport) ULONG EventWrite(
        REGHANDLE              RegHandle,
        PCEVENT_DESCRIPTOR     EventDescriptor,
        ULONG                  UserDataCount,
        PEVENT_DATA_DESCRIPTOR UserData
    )
    {
        for (int i = 0; i < UserDataCount; i++)
        {
            PEVENT_DATA_DESCRIPTOR Current = UserData + i;
            std::ofstream logFile;
            logFile.open("C:\\GlanceTracing.txt", std::ofstream::out | std::ofstream::app);

            char* realBuffer = (char*)malloc(sizeof(char) * Current->Size);
            wchar_t* buffer = (wchar_t*)malloc(sizeof(wchar_t) * Current->Size);
            RtlCopyMemory(buffer, (wchar_t*)(ULONG_PTR)Current->Ptr, Current->Size);
            size_t Converted = 0;
            wcstombs_s(&Converted, realBuffer, (size_t)(sizeof(char) * Current->Size), buffer, (size_t)(sizeof(wchar_t) * Current->Size));

            logFile << "Service Message[" << i << "-" << Current->Size << "]: " << realBuffer << "\n";

            logFile.close();
            free(buffer);
            free(realBuffer);
        }

        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventWrite pEventWrite = (PEventWrite)GetProcAddress(Dll, "EventWrite");
        ULONG result = pEventWrite(RegHandle, EventDescriptor, UserDataCount, UserData);

        return result;
    }

    __declspec(dllexport) ULONG EventWriteTransfer(
        REGHANDLE              RegHandle,
        PCEVENT_DESCRIPTOR     EventDescriptor,
        LPCGUID                ActivityId,
        LPCGUID                RelatedActivityId,
        ULONG                  UserDataCount,
        PEVENT_DATA_DESCRIPTOR UserData
    )
    {
        for (int i = 0; i < UserDataCount; i++)
        {
            PEVENT_DATA_DESCRIPTOR Current = UserData + i;
            std::ofstream logFile;
            logFile.open("C:\\GlanceTracing.txt", std::ofstream::out | std::ofstream::app);

            char* realBuffer = (char*)malloc(sizeof(char) * Current->Size);
            wchar_t* buffer = (wchar_t*)malloc(sizeof(wchar_t) * Current->Size);
            RtlCopyMemory(buffer, (wchar_t*)(ULONG_PTR)Current->Ptr, Current->Size);
            size_t Converted = 0;
            wcstombs_s(&Converted, realBuffer, (size_t)(sizeof(char) * Current->Size), buffer, (size_t)(sizeof(wchar_t) * Current->Size));

            logFile << "Service Message[" << i << "-" << Current->Size << "]: " << realBuffer << "\n";

            logFile.close();
            free(buffer);
            free(realBuffer);
        }

        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventWriteTransfer pEventWriteTransfer = (PEventWriteTransfer)GetProcAddress(Dll, "EventWriteTransfer");
        ULONG result = pEventWriteTransfer(RegHandle, EventDescriptor, ActivityId, RelatedActivityId, UserDataCount, UserData);

        return result;
    }

    __declspec(dllexport) ULONG EventSetInformation(
        REGHANDLE        RegHandle,
        EVENT_INFO_CLASS InformationClass,
        PVOID            EventInformation,
        ULONG            InformationLength
    )
    {
        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventSetInformation pEventWriteTransfer = (PEventSetInformation)GetProcAddress(Dll, "EventSetInformation");
        ULONG result = pEventWriteTransfer(RegHandle, InformationClass, EventInformation, InformationLength);

        return result;
    }

    __declspec(dllexport) ULONG EventUnregister(
        REGHANDLE RegHandle
    )
    {
        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventUnregister pEventWriteTransfer = (PEventUnregister)GetProcAddress(Dll, "EventUnregister");
        ULONG result = pEventWriteTransfer(RegHandle);

        return result;
    }

    __declspec(dllexport) ULONG EventRegister(
        LPCGUID         ProviderId,
        PENABLECALLBACK EnableCallback,
        PVOID           CallbackContext,
        PREGHANDLE      RegHandle
    )
    {
        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventRegister pEventWriteTransfer = (PEventRegister)GetProcAddress(Dll, "EventRegister");
        ULONG result = pEventWriteTransfer(ProviderId, EnableCallback, CallbackContext, RegHandle);

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

