#pragma once

#include "pch.h"
#include <iostream>//std::out/in c.out, c.in : libraries C++
#include <stdio.h> // librairie C (printf...)

#include <gcroot.h> // .NET garbage collector
#include <windows.h> // windows API
#include <vcclr.h> // For GCHandle
#include <tchar.h>

#using <System.dll>
#using <mscorlib.dll>

using namespace System;
using namespace System::Threading;

//namespace ClientsComCppCLI
//{
//	static public ref class Class1
//	{
//	public:
//		static long Add(long a, long b)
//		{
//			long* ptr;
//			ptr = &a;
//			
//			return(*ptr + b);
//		}
//	};
//}

namespace ClientsComCppCLI
{
	// Define a structure to hold the shared data
	//struct SharedData
	//{
	//	int syncValue = 0;
	//	long Data;
	//	// Add other fields as needed
	//};

	static public ref class ListManagement // add a ref if managed, otherwise it is considered unmanaged
	{
	public:
		static void Communication()
		{
			LPVOID pBuf;
			HANDLE hMapFile;
			HANDLE hTimer = NULL;
			TCHAR cName[] = TEXT("FileMappingObject");
			const size_t BufSize = 1024;
			const long nThreadId = (long)GetCurrentThreadId();
			long nCounter = 0;
			bool bEnter = false;
			int nCallCount = 0;
			static const int nSyncValueSize = sizeof(int);

			// Timer function
			void  State(Object ^ state);
			{
				Console::WriteLine("Call per s.: " + nCounter);

				// Perform atomic reset of nCounter
				nCounter = 0;
			}


			void CleanUp(void);
			{
				if (hTimer) DeleteTimerQueueTimer(NULL, hTimer, NULL);
				UnmapViewOfFile(pBuf);
				if (hMapFile == NULL) CloseHandle(hMapFile);
			}

			void CALLBACK State(void* lpParametar, BOOLEAN TimerOrWaitFired);
			{
				printf("Call per s.: %d\n", nCounter);
				nCounter = 0;
			}

			// Initiate the Timer
			TimerCallback^ timer = gcnew TimerCallback(&State);
			Timer^ tState = gcnew Timer(timer, nullptr, TimeSpan::FromSeconds(1), TimeSpan::FromSeconds(1));

			if (!	CreateTimerQueueTimer(&hTimer, NULL,
			(WAITORTIMERCALLBACK)State, NULL, 1000, 1000, 0);)
			{
				printf("Timer creation failed %d\n", GetLastError());
				CleanUp();
			}

			//Create Shared Memory
			//When you call CreateFileMapping with INVALID_HANDLE_VALUE as the file handle, and the name 
			//you specify (cName) already exists, the function will open the existing mapping instead of 
			//creating a new one
			hMapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,    // use paging file
				NULL,                    // default security
				PAGE_READWRITE,          // read/write access
				0,                       // maximum object size (high-order DWORD)
				BufSize,                // maximum object size (low-order DWORD)
				cName);                 // name of mapping object
			if (hMapFile == NULL)
			{
				printf("Could not open file mapping object %d\n", GetLastError());
				CleanUp();
			}

			// Map the shared memory to the struct type
			pBuf = MapViewOfFile(hMapFile,	// handle to map object
				FILE_MAP_ALL_ACCESS,		// read/write permission
				0,
				0,
				BufSize);
			if (pBuf == NULL)
			{
				printf("Could not map view of file %d\n", GetLastError());
				CleanUp();
			}

			//// Cast the mapped memory to the SharedData struct
			//SharedData* pData = static_cast<SharedData*>(pBuf);
			// Create the shared memory array
			int* aSharedArray = static_cast<int*>(pBuf);

			while (true)
			{
				// Now we deal with the Synchronization
				if (System::Threading::Interlocked::CompareExchange(aSharedArray[0], 1, 0) == 0)
				{
					bEnter = true;
				}

				// Synchronisation completed, we can enter 
				if (bEnter)
				{
					bEnter = false;

					// Print the content of shared memory
					printf("Data read : %ld\n", aSharedArray[1]);

					// Change the content of the shared memory
					//pData->Data = nThreadId; // Assign nThreadId to the syncValue field
					aSharedArray[1] = nThreadId;


					//*(long*)pBuf = nThreadId;
				}

				// Release synchronization
				//pData->syncValue = 0;
				aSharedArray[0] = 0;

				nCounter++;
			};
		};
	};
};
