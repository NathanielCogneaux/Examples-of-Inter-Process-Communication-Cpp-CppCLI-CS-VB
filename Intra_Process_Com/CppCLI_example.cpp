#include "pch.h"
#include <iostream>
#include <gcroot.h>

using namespace System;
using namespace System::Threading;

// Global variables
long nCounter = 0;
long nShareMemory = 0;

gcroot<AutoResetEvent^> eClientWicket = gcnew AutoResetEvent(true);
gcroot<AutoResetEvent^> eServerWicket = gcnew AutoResetEvent(false);

//AutoResetEvent eClientWicket(true);
//AutoResetEvent eServerWicket(false);

bool bWeOff = false;

// Timer function
void State(Object^ state)
{
    Console::WriteLine("Call per s.: " + nCounter);

    // Perform atomic reset of nCounter
    nCounter = 0;
}

// Client (writer / applicant) thread function
void Client()
{
    do
    {
        // The wicket is initially open (signaled) and will close automatically after passing through (AutoResetEvent)
        eClientWicket->WaitOne();

        // Something to do
        nShareMemory++;

        // Open the server wicket
        eServerWicket->Set();

    } while (!bWeOff);

    Console::WriteLine("Client thread exit");
}

// Server (reader / counter clerk) thread function
void Server()
{
    do
    {
        // The wicket is initially closed (unsignaled) and will automatically close after passing through (AutoResetEvent)
        eServerWicket->WaitOne();

        // Some important job...
        if (nShareMemory == 100000)
        {
            nShareMemory = 0;
        }

        // Service counter (possible concurrency with State)
        nCounter++;

        // Open the client wicket
        eClientWicket->Set();

    } while (!bWeOff);

    Console::WriteLine("Server thread exit");
}

int main()
{
    // Timer launch
    TimerCallback^ timer = gcnew TimerCallback(&State);
    Timer^ tState = gcnew Timer(timer, nullptr, TimeSpan::FromSeconds(1), TimeSpan::FromSeconds(1));

    // Client (writer / applicant) thread launch
    Thread^ tClient = gcnew Thread(gcnew ThreadStart(&Client));
    tClient->Start();

    // Server (reader / counter clerk) thread launch
    Thread^ tServer = gcnew Thread(gcnew ThreadStart(&Server));
    tServer->Start();

    // Wait for Enter
    Console::ReadLine();

    bWeOff = true;

    Console::WriteLine("Main exit");

    return 0;
}
