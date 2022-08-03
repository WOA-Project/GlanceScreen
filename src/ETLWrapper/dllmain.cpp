// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "dllmain2.h"
#include <stdlib.h>
#include <fstream>

typedef ULONG(WINAPI *PEventWrite)(
    REGHANDLE RegHandle,
    PCEVENT_DESCRIPTOR EventDescriptor,
    ULONG UserDataCount,
    PEVENT_DATA_DESCRIPTOR UserData);

typedef ULONG(WINAPI *PEventWriteTransfer)(
    REGHANDLE RegHandle,
    PCEVENT_DESCRIPTOR EventDescriptor,
    LPCGUID ActivityId,
    LPCGUID RelatedActivityId,
    ULONG UserDataCount,
    PEVENT_DATA_DESCRIPTOR UserData);

typedef ULONG(WINAPI *PEventSetInformation)(
    REGHANDLE RegHandle,
    EVENT_INFO_CLASS InformationClass,
    PVOID EventInformation,
    ULONG InformationLength);

typedef ULONG(WINAPI *PEventRegister)(
    LPCGUID ProviderId,
    PENABLECALLBACK EnableCallback,
    PVOID CallbackContext,
    PREGHANDLE RegHandle);

typedef ULONG(WINAPI *PEventUnregister)(
    REGHANDLE RegHandle);

typedef ULONG(WINAPI *PEventRegister)(
    LPCGUID ProviderId,
    PENABLECALLBACK EnableCallback,
    PVOID CallbackContext,
    PREGHANDLE RegHandle);

ULONG ReadAndPrintArrayData(UINT32 DataType, ULONG_PTR Data, ULONG DataSize);
ULONG ReadAndPrintData(UINT32 DataType, ULONG_PTR Data, ULONG DataSize);

ULONG ReadAndPrintArrayData(UINT32 DataType, ULONG_PTR Data, ULONG DataSize)
{
    UINT16 ArraySize = *(UINT16 *)Data;

    ULONG_PTR ArrayData = Data + sizeof(UINT16);

    ULONG ReadSize = 0;

    std::ofstream logFile;
    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);
    logFile << "[";
    logFile.close();

    for (UINT16 Element = 0; Element < ArraySize; Element++)
    {
        ReadSize += ReadAndPrintData(DataType, ArrayData, DataSize);
        ArrayData = Data + sizeof(UINT16) + ReadSize;

        logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);
        logFile << ",";
        logFile.close();
    }

    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);
    logFile << "]";
    logFile.close();

    return ReadSize;
}

ULONG ReadAndPrintData(UINT32 DataType, ULONG_PTR Data, ULONG DataSize)
{
    std::ofstream logFile;
    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

    ULONG readBytes = DataSize;

    switch (DataType & 0x1f)
    {
    case 1:
    {
        // Unicode String
        PWCHAR el = (PWCHAR)Data;
        ULONG strSize = wcslen(el);
        ULONG bufSize = wcslen(el) * sizeof(wchar_t) + 2;
        ULONG cBufSize = strSize + 1;

        char *realBuffer = (char *)malloc(sizeof(char) * cBufSize);

        size_t Converted = 0;
        wcstombs_s(&Converted, realBuffer, cBufSize, el, bufSize);

        logFile << realBuffer;

        free(realBuffer);
        readBytes = bufSize;
        break;
    }
    case 2:
    {
        // ASCII String
        PCHAR el = (PCHAR)Data;

        logFile << el;
        readBytes = strlen(el);
        break;
    }
    case 23:
    {
        // ASCII + Count
        CHAR Size = *(CHAR *)Data;

        PCHAR el = (PCHAR)malloc(Size + 1);
        memset(el, 0, Size + 1);
        memcpy(el, (PCHAR)(Data + 1), Size);

        // print here

        logFile << el;

        free(el);
        readBytes = Size;
        break;
    }

    case 3:
    {
        // INT8
        INT8 el = *(INT8 *)Data;

        logFile << el;
        readBytes = sizeof(INT8);
        break;
    }
    case 4:
    {
        // UINT8
        UINT8 el = *(UINT8 *)Data;

        logFile << el;
        readBytes = sizeof(UINT8);
        break;
    }
    case 5:
    {
        // INT16
        INT16 el = *(INT16 *)Data;

        logFile << el;
        readBytes = sizeof(INT16);
        break;
    }
    case 6:
    {
        // UINT16
        UINT16 el = *(UINT16 *)Data;

        logFile << el;
        readBytes = sizeof(UINT16);
        break;
    }
    case 7:
    {
        // INT32
        INT32 el = *(INT32 *)Data;

        logFile << el;
        readBytes = sizeof(INT32);
        break;
    }
    case 8:
    {
        // UINT32
        UINT32 el = *(UINT32 *)Data;

        logFile << el;
        readBytes = sizeof(UINT32);
        break;
    }
    case 20:
    {
        // HEXINT32
        UINT32 el = *(UINT32 *)Data;

        logFile << el;
        readBytes = sizeof(UINT32);
        break;
    }
    case 9:
    {
        // INT64
        INT64 el = *(INT64 *)Data;

        logFile << el;
        readBytes = sizeof(INT64);
        break;
    }
    case 10:
    {
        // UINT64
        UINT64 el = *(UINT64 *)Data;

        logFile << el;
        readBytes = sizeof(UINT64);
        break;
    }
    case 21:
    {
        // HEXINT64
        UINT64 el = *(UINT64 *)Data;

        logFile << el;
        readBytes = sizeof(UINT64);
        break;
    }
    case 11:
    {
        // FLOAT
        FLOAT el = *(FLOAT *)Data;

        logFile << el;
        readBytes = sizeof(FLOAT);
        break;
    }
    case 12:
    {
        // DOUBLE
        double el = *(double *)Data;

        logFile << el;
        readBytes = sizeof(double);
        break;
    }
    case 13:
    {
        // BOOL32
        DWORD el = *(DWORD *)Data;

        logFile << el;
        readBytes = sizeof(DWORD);
        break;
    }
    case 14:
    {
        // BINARY
        // Array eh
        readBytes = ReadAndPrintArrayData(4, Data, DataSize);
        break;
    }
    case 15:
    {
        // GUID
        GUID el = *(GUID *)Data;

        // logFile << el; //TODO!

        readBytes = sizeof(GUID);
        break;
    }
    case 18:
    {
        // SYSTEMTIME
        SYSTEMTIME el = *(SYSTEMTIME *)Data;

        // logFile << el; //TODO!

        readBytes = sizeof(SYSTEMTIME);
        break;
    }
    }

    logFile.close();

    return readBytes;
}

extern "C"
{
    __declspec(dllexport) ULONG EventWrite(
        REGHANDLE RegHandle,
        PCEVENT_DESCRIPTOR EventDescriptor,
        ULONG UserDataCount,
        PEVENT_DATA_DESCRIPTOR UserData)
    {
        std::ofstream logFile;
        /*for (int i = 0; i < UserDataCount; i++)
        {
            PEVENT_DATA_DESCRIPTOR Current = UserData + i;
            std::ofstream logFile;
            logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

            char* realBuffer = (char*)malloc(sizeof(char) * Current->Size);
            wchar_t* buffer = (wchar_t*)malloc(sizeof(wchar_t) * Current->Size);
            RtlCopyMemory(buffer, (wchar_t*)(ULONG_PTR)Current->Ptr, Current->Size);
            size_t Converted = 0;
            wcstombs_s(&Converted, realBuffer, (size_t)(sizeof(char) * Current->Size), buffer, (size_t)(sizeof(wchar_t) * Current->Size));

            logFile << "Service Message[" << i << "-" << Current->Size << "]: " << realBuffer << "\n";

            logFile.close();
            free(buffer);
            free(realBuffer);
        }*/

        for (ULONG i = 0; i < UserDataCount; i++)
        {
            PEVENT_DATA_DESCRIPTOR Current = UserData + i;

            if (Current->Reserved == 2)
            {
                // Event GUID
                continue;
            }

            if (Current->Reserved == 1)
            {
                ULONG_PTR ptr = (ULONG_PTR)Current->Ptr;

                // Event Name + var names
                UINT16 StructSize = *(UINT16 *)ptr;
                ptr += sizeof(UINT16);

                UCHAR Tag = *(UCHAR *)ptr;
                ptr += sizeof(UCHAR);

                if (Tag & 0x80)
                {
                    ptr += sizeof(UCHAR); // Unknown again
                }

                CHAR *Name = (CHAR *)ptr;
                ptr += sizeof(CHAR) * (strlen(Name) + 1);

                logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                logFile << "[" << Name << "]: ";

                logFile.close();

                // UINT16 DataFieldsSize = Current->Ptr - ptr;

                while (ptr < (ULONG_PTR)Current->Ptr + StructSize)
                {
                    CHAR *DataFieldName = (CHAR *)ptr;
                    ptr += sizeof(CHAR) * (strlen(DataFieldName) + 1);

                    UCHAR TagIn = *(UCHAR *)ptr;
                    ptr += sizeof(UCHAR);

                    UCHAR TagOut = 0;

                    if (TagIn & 128)
                    {
                        TagOut = *(UCHAR *)ptr;
                        ptr += sizeof(UCHAR);

                        if (TagOut & 0x80)
                        {
                            ptr += sizeof(UINT32); // Unknown
                        }
                    }

                    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                    logFile << DataFieldName << "=";

                    logFile.close();

                    i++;
                    PEVENT_DATA_DESCRIPTOR DataField = UserData + i;
                    ULONG_PTR Data = DataField->Ptr;
                    ULONG DataSize = DataField->Size;

                    UINT32 DataType = TagIn & 0x1f;

                    if (TagIn & 32 || TagIn & 64)
                    {
                        ReadAndPrintArrayData(DataType, Data, DataSize);
                    }
                    else
                    {
                        ReadAndPrintData(DataType, Data, DataSize);
                    }

                    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                    logFile << ", ";

                    logFile.close();
                }

                logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                logFile << "\n";

                logFile.close();
            }
        }

        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventWrite pEventWrite = (PEventWrite)GetProcAddress(Dll, "EventWrite");
        ULONG result = pEventWrite(RegHandle, EventDescriptor, UserDataCount, UserData);

        return result;
    }

    __declspec(dllexport) ULONG EventWriteTransfer(
        REGHANDLE RegHandle,
        PCEVENT_DESCRIPTOR EventDescriptor,
        LPCGUID ActivityId,
        LPCGUID RelatedActivityId,
        ULONG UserDataCount,
        PEVENT_DATA_DESCRIPTOR UserData)
    {
        /*for (int i = 0; i < UserDataCount; i++)
        {
            PEVENT_DATA_DESCRIPTOR Current = UserData + i;
            std::ofstream logFile;
            logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

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

        return result;*/

        std::ofstream logFile;

        for (ULONG i = 0; i < UserDataCount; i++)
        {
            PEVENT_DATA_DESCRIPTOR Current = UserData + i;

            if (Current->Reserved == 2)
            {
                // Event GUID
                continue;
            }

            if (Current->Reserved == 1)
            {
                ULONG_PTR ptr = (ULONG_PTR)Current->Ptr;

                // Event Name + var names
                UINT16 StructSize = *(UINT16 *)ptr;
                ptr += sizeof(UINT16);

                UCHAR Tag = *(UCHAR *)ptr;
                ptr += sizeof(UCHAR);

                if (Tag & 0x80)
                {
                    ptr += sizeof(UCHAR); // Unknown again
                }

                CHAR *Name = (CHAR *)ptr;
                ptr += sizeof(CHAR) * (strlen(Name) + 1);

                logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                logFile << "[" << Name << "]: ";

                logFile.close();

                // UINT16 DataFieldsSize = Current->Ptr - ptr;

                while (ptr < (ULONG_PTR)Current->Ptr + StructSize)
                {
                    CHAR *DataFieldName = (CHAR *)ptr;
                    ptr += sizeof(CHAR) * (strlen(DataFieldName) + 1);

                    UCHAR TagIn = *(UCHAR *)ptr;
                    ptr += sizeof(UCHAR);

                    UCHAR TagOut = 0;

                    if (TagIn & 128)
                    {
                        TagOut = *(UCHAR *)ptr;
                        ptr += sizeof(UCHAR);

                        if (TagOut & 0x80)
                        {
                            ptr += sizeof(UINT32); // Unknown
                        }
                    }

                    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                    logFile << DataFieldName << "=";

                    logFile.close();

                    i++;
                    PEVENT_DATA_DESCRIPTOR DataField = UserData + i;
                    ULONG_PTR Data = DataField->Ptr;
                    ULONG DataSize = DataField->Size;

                    UINT32 DataType = TagIn & 0x1f;

                    if (TagIn & 32 || TagIn & 64)
                    {
                        ReadAndPrintArrayData(DataType, Data, DataSize);
                    }
                    else
                    {
                        ReadAndPrintData(DataType, Data, DataSize);
                    }

                    logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                    logFile << ", ";

                    logFile.close();
                }

                logFile.open("C:\\TouchProcessorTracing.txt", std::ofstream::out | std::ofstream::app);

                logFile << "\n";

                logFile.close();
            }
        }

        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventWriteTransfer pEventWriteTransfer = (PEventWriteTransfer)GetProcAddress(Dll, "EventWriteTransfer");
        ULONG result = pEventWriteTransfer(RegHandle, EventDescriptor, ActivityId, RelatedActivityId, UserDataCount, UserData);

        return result;
    }

    __declspec(dllexport) ULONG EventSetInformation(
        REGHANDLE RegHandle,
        EVENT_INFO_CLASS InformationClass,
        PVOID EventInformation,
        ULONG InformationLength)
    {
        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventSetInformation pEventWriteTransfer = (PEventSetInformation)GetProcAddress(Dll, "EventSetInformation");
        ULONG result = pEventWriteTransfer(RegHandle, InformationClass, EventInformation, InformationLength);

        return result;
    }

    __declspec(dllexport) ULONG EventUnregister(
        REGHANDLE RegHandle)
    {
        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventUnregister pEventWriteTransfer = (PEventUnregister)GetProcAddress(Dll, "EventUnregister");
        ULONG result = pEventWriteTransfer(RegHandle);

        return result;
    }

    __declspec(dllexport) ULONG EventRegister(
        LPCGUID ProviderId,
        PENABLECALLBACK EnableCallback,
        PVOID CallbackContext,
        PREGHANDLE RegHandle)
    {
        HMODULE Dll = LoadLibrary(L"advapi32.dll");
        PEventRegister pEventWriteTransfer = (PEventRegister)GetProcAddress(Dll, "EventRegister");
        ULONG result = pEventWriteTransfer(ProviderId, EnableCallback, CallbackContext, RegHandle);

        return result;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
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
