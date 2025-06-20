#include <windows.h>
#include <iostream> //std::out/in c.out, c.in : libraries C++
#include <stdio.h> // librairie C (printf...)
#include <conio.h>
#include <tchar.h>

HANDLE ghClientEvent;
HANDLE ghServerEvent;

LPVOID pBuf;
HANDLE hMapFile;

DWORD dwWaitResult;

#define BUF_SIZE 256
TCHAR szName[] = TEXT("MyFileMappingObject"); //Global

//DWORD dwThreadID;

VOID CleanUp(void)
{
    //if (hTimer) DeleteTimerQueueTimer(NULL, hTimer, NULL);
    UnmapViewOfFile(pBuf);
    if (hMapFile) CloseHandle(hMapFile);

    if (ghClientEvent) CloseHandle(ghClientEvent);
    if (ghServerEvent) CloseHandle(ghServerEvent);

    //if (ghClientThread) CloseHandle(ghClientThread);
    //if (ghServerThread) CloseHandle(ghServerThread);
}


int main()
{
    // Signaled client event
    if (!(ghClientEvent = CreateEvent(
        NULL,						// default security attributes
        FALSE,						// auto-reset event
        TRUE,						// initial state is signaled
        TEXT("ClientEvent")			// object name
    )))
    {
        printf("Client event creation failed (%d)\n", GetLastError());
        return 1;
    }

    // Unsignaled server event
    if (!(ghServerEvent = CreateEvent(
        NULL,						// default security attributes
        FALSE,						// manual-reset event
        FALSE,						// initial state is nonsignaled
        TEXT("ServerEvent")			// object name
    )))
    {
        printf("Server event creation failed (%d)\n", GetLastError());
        CleanUp();
        return 1;
    }

    while (true)
    {
        // Wait for the client event
        dwWaitResult = WaitForSingleObject(
            ghClientEvent,						// event handle
            INFINITE);							// indefinite wait


        //Open Shared Memory
        hMapFile = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,   // read/write access
            FALSE,                 // do not inherit the name
            szName);               // name of mapping object
        if (hMapFile == NULL)//Shared Memory does not exist
        {
            _tprintf(TEXT("Could not open file mapping object (%d).\n"),
                GetLastError());
            CleanUp();
            return 1;
        }

        pBuf = MapViewOfFile(hMapFile, // handle to map object
            FILE_MAP_ALL_ACCESS,  // read/write permission
            0,
            0,
            BUF_SIZE);
        if (pBuf == NULL) //
        {
            _tprintf(TEXT("Could not map view of file (%d).\n"),
                GetLastError());
            CleanUp();
            return 1;
        }

        _tprintf(TEXT("Received message: %s\n"), (LPTSTR)pBuf);

        _tcscpy_s((LPTSTR)pBuf, BUF_SIZE, TEXT("hi\tblabla"));

        //_tprintf(TEXT("Sending message: %s\n"), pBuf);

        // Signal the server
        SetEvent(ghServerEvent);

        Sleep(1000);
    }

    // Wait for Enter
    std::cin.ignore();
    CleanUp();
    return 0;
}




// Wait for the server to process the message
//while (_tcslen((LPTSTR)pBuf) > 0)
//{
//    Sleep(100);
//}

//#include <thread> // For std::this_thread::sleep_for
//std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait for 5 seconds before sending the next message

        //_tprintf(TEXT("Enter a message to send: "));
        //_getts_s((LPTSTR)pBuf, BUF_SIZE);