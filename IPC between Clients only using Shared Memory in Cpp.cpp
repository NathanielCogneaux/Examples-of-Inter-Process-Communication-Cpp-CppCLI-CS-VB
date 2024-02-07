#include <windows.h>
#include <stdio.h>	// librairie C
#include <conio.h>
#include <tchar.h>

long nCounter;
bool bWeOff;

HANDLE hTimer = NULL;
void CALLBACK State(void* lpParametar, BOOLEAN TimerOrWaitFired);

LPVOID pBuf;
HANDLE hMapFile;

#define BUF_SIZE 256
TCHAR cName[] = TEXT("MyFileMappingObject"); //Global

void CALLBACK State(void* lpParametar, BOOLEAN TimerOrWaitFired)
{
	printf("Call per s.: %d\n", nCounter);
	nCounter = 0;

	// To exit
	if (_kbhit()) {
		bWeOff = true;
	}
}

VOID CleanUp(void)
{
	if (hTimer) DeleteTimerQueueTimer(NULL, hTimer, NULL);

	// Set mutex
	((long*)pBuf)[0] = 0;

	UnmapViewOfFile(pBuf);
	if (hMapFile) CloseHandle(hMapFile);
}

BOOL sig_handler(DWORD signum)
{
	switch (signum)
	{
	case CTRL_CLOSE_EVENT:
		CleanUp();
		return(TRUE);

	default:
		return FALSE;
	}
}

int main()
{
	// Cleanup on console close

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)sig_handler, TRUE);

	// Timer
	if (!CreateTimerQueueTimer(&hTimer, NULL,
		(WAITORTIMERCALLBACK)State, NULL, 1000, 1000, 0)) // not null
	{
		printf("Timer creation failed %d\n", GetLastError());
		return 1;
	}

	//Open Shared Memory
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	// read/write access
		FALSE,					// do not inherit the name
		cName);				// name of mapping object
	if (hMapFile == NULL)		//Shared Memory does not exist
	{
		printf("Could not open file mapping object %d\n",GetLastError());
		CleanUp();
		return 1;
	}

	pBuf = MapViewOfFile(hMapFile,	// handle to map object
		FILE_MAP_ALL_ACCESS,		// read/write permission
		0,
		0,
		BUF_SIZE);
	if (pBuf == NULL)
	{
		printf("Could not map view of file %d\n",GetLastError());
		CleanUp();
		return 1;
	}

	// Main loop
	for (;;) {

		// Spinner (the _ is for the intrisic)
		while (!_InterlockedCompareExchange(&((long*)pBuf)[0], 1, 0)) {}

		// Shared memory part
		//Sleep(250);
		((long*)pBuf)[1]++;

		// Count
		nCounter++;

		// Release the "mutex" after communication
		((long*)pBuf)[0] = 0;

		// _kbhit musn't be tested in the main loop!
		if (bWeOff) {
			break;
		}
	}

	CleanUp();
	return 0;
}
