//inter process - server créé memoire partagée + events, le serveur créé tout bcs multiple clients -- mutex pour multiple clients
// shared memory cette fois ci, definition générale, recuperation de variables

#include <windows.h>
#include <stdio.h> // librairie C (printf...)
#include <iostream> //std::out/in c.out, c.in : libraries C++


long nCounter;
long nShareMemory;

HANDLE ghClientThread;
HANDLE ghServerThread;

DWORD WINAPI Client(LPVOID lpParam);
DWORD WINAPI Server(LPVOID lpParam);
void CALLBACK State(void* lpParametar, BOOLEAN TimerOrWaitFired);

bool lWeOff;

HANDLE hTimer = NULL;

VOID CleanUp(void)
{
	if (hTimer) DeleteTimerQueueTimer(NULL, hTimer, NULL);

	//if (ghClientEvent) CloseHandle(ghClientEvent);
	//if (ghServerEvent) CloseHandle(ghServerEvent);

	if (ghClientThread) CloseHandle(ghClientThread);
	if (ghServerThread) CloseHandle(ghServerThread);
}

void CALLBACK State(void* lpParametar, BOOLEAN TimerOrWaitFired)
{
	printf("Call per s.: %d\n", nCounter);
	nCounter = 0;
}


int main()
{
	int arg;
	char c;
	DWORD dwThreadID;

	// Timer
	if (!CreateTimerQueueTimer(&hTimer, NULL,
		(WAITORTIMERCALLBACK)State, NULL, 1000, 1000, 0)) // not null
	{
		printf("Timer creation failed (%d)\n", GetLastError());
		return 1;
	}

	// Server thread
	if (!(ghServerThread = CreateThread(
		NULL,						// default security
		0,							// default stack size
		Server,						// name of the thread function
		NULL,						// no thread parameters
		0,							// default startup flags
		&dwThreadID)))
	{
		printf("Server thread creation failed (%d)\n", GetLastError());
		CleanUp();
		return 1;
	}

	Sleep(1000);

	// Client thread
	if (!(ghClientThread = CreateThread(
		NULL,						// default security
		0,							// default stack size
		Client,						// name of the thread function
		NULL,						// no thread parameters
		0,							// default startup flags
		&dwThreadID)))
	{
		printf("Client thread creation failed (%d)\n", GetLastError());
		CleanUp();
		return 1;
	}

	// Wait for Enter
	std::cin.ignore();

	lWeOff = true;

	CleanUp();

	return 0;
}

DWORD WINAPI Client(LPVOID lpParam)
{
	HANDLE ghClientEvent;
	HANDLE ghServerEvent;

	DWORD dwWaitResult;

	// Open the Signaled client event created by the Server process
	if (!(ghClientEvent = OpenEvent(
		EVENT_ALL_ACCESS,			// default security attributes
		FALSE,						
		TEXT("Global\\ClientEvent")			// object name
	)))
	{
		printf("Can't access Client Event (%d)\n", GetLastError());
		CleanUp();
		return 1;
	}

	// Open the Unsignaled server event created by the Server process
	if (!(ghServerEvent = OpenEvent(
		EVENT_ALL_ACCESS,			// default security attributes
		FALSE,						
		TEXT("Global\\ServerEvent")			// object name
	)))
	{
		printf("Can't access Server Event (%d)\n", GetLastError());
		CleanUp();
		return 1;
	}

	do {
		// Wait for the client event
		dwWaitResult = WaitForSingleObject(
			ghClientEvent,						// event handle
			INFINITE);							// indefinite wait

		//int nLastError = GetLastError();
		nShareMemory++;

		// Signal the server
		SetEvent(ghServerEvent);

	} while (!lWeOff);

	printf("Client thread exit\n");

	return 0;
}

DWORD WINAPI Server(LPVOID lpParam)
{
	HANDLE ghClientEvent;
	HANDLE ghServerEvent;

	DWORD dwWaitResult;

	// Signaled client event
	if (!(ghClientEvent = CreateEvent(
		NULL,						// default security attributes
		FALSE,						// auto-reset event
		TRUE,						// initial state is signaled
		TEXT("Global\\ClientEvent")			// object name
	)))
	{
		printf("Client event creation failed (%d)\n", GetLastError());
		CleanUp();
		return 1;
	}

	// Unsignaled server event
	if (!(ghServerEvent = CreateEvent(
		NULL,						// default security attributes
		FALSE,						// auto-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("Global\\ServerEvent")			// object name
	)))
	{
		printf("Server event creation failed (%d)\n", GetLastError());
		CleanUp();
		return 1;
	}

	do {
		// Wait for the server event
		dwWaitResult = WaitForSingleObject(
			ghServerEvent,						// event handle
			INFINITE);							// indefinite wait

		// The important job...
		if (nShareMemory == 100000)
		{
			nShareMemory = 0;
		}

		// Service counter
		nCounter++;

		// Signal the client
		SetEvent(ghClientEvent);

	} while (!lWeOff);

	if (ghClientEvent) CloseHandle(ghClientEvent);
	if (ghServerEvent) CloseHandle(ghServerEvent);

	printf("Server thread exit\n");

	return 0;
}