//Launch the server first

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
TCHAR szName[] = TEXT("MyFileMappingObject");

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
	//Create Shared Memory
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		szName);                 // name of mapping object
	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	pBuf = MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		BUF_SIZE);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());
		CleanUp();
		return 1;
	}

	// Signaled client event
	if (!(ghClientEvent = CreateEvent(
		NULL,						// default security attributes
		FALSE,						// auto-reset event
		TRUE,						// initial state is signaled
		TEXT("ClientEvent")			// object name
	)))
	{
		printf("Client event creation failed (%d)\n", GetLastError());
		CleanUp();
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

	//Once everything is created - wait for a client to show up
	while (true) 
	{
		// Wait for the server event - event handle - indefinite wait
		dwWaitResult = WaitForSingleObject(ghServerEvent, INFINITE);

		_tprintf(TEXT("Received message: %s\n"), (LPTSTR)pBuf);

		//_tprintf(TEXT("Permission Granted sent"));

		// Clear the buffer to signal the message has been processed ""
		_tcscpy_s((LPTSTR)pBuf, BUF_SIZE, TEXT("Permission Granted"));

		// Signal the client
		SetEvent(ghClientEvent);

		Sleep(1000); // Give some time before checking again
	}

	// Wait for Enter
	std::cin.ignore();
	CleanUp();
	return 0;
}



// Check if a message has been received
//if (_tcslen((LPTSTR)pBuf) > 0)