using System;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Text;
using System.Threading;
using Newtonsoft.Json;

using ClientsComCppCLI;

class Message
{
    public string content;
}

class OrderApply
{
    static void Main(string[] args)
    {
        ListManagement.Communication();
    }
}


static public long nCallCount = 0;
const int nMaxSize = 1024;
const int nSyncValueSize = 4; // Size of int in bytes

static public void Timer(object sender)
{
    Console.WriteLine($"Calls per second : {nCallCount:F2}");
    nCallCount = 0;
}


bool bEnter = false;

// initialise the timer
Timer tState = new Timer(new TimerCallback(Timer), null, new TimeSpan(0), new TimeSpan(0, 0, 1));

int nClientId = Thread.GetCurrentProcessorId();

// this is what we want to write to the memory mapped file
Message message = new Message();
message.content = nClientId.ToString();

using (MemoryMappedFile oInterlocked_Mutex_Memory = MemoryMappedFile.CreateOrOpen("Sync_Management", nSyncValueSize, MemoryMappedFileAccess.ReadWrite))
using (MemoryMappedViewAccessor mmvSync = oInterlocked_Mutex_Memory.CreateViewAccessor(0, nSyncValueSize))

using (MemoryMappedFile oMemMapFile = MemoryMappedFile.CreateOrOpen("ClientsSharedMemory", nMaxSize, MemoryMappedFileAccess.ReadWrite))
using (MemoryMappedViewAccessor mmvMemory = oMemMapFile.CreateViewAccessor(0, nMaxSize))

{
    byte[] buffer = new byte[nMaxSize];

    while (true)
    {
        //int nEnterValue = mmvSync.ReadInt32(0);
        unsafe
        {
            byte* ptr = null;

            // Get a pointer into the unmanaged memory of the view. 
            mmvSync.SafeMemoryMappedViewHandle.AcquirePointer(ref ptr);

            if (ptr != null) // Memory not accessible
            {
                if (Interlocked.CompareExchange(ref *(int*)ptr, 1, 0) == 0)
                {
                    bEnter = true;
                }
                mmvSync.SafeMemoryMappedViewHandle.ReleasePointer();
            }
        }

        if (bEnter)
        {
            bEnter = false;
            // First - write Serialized data in the Memory
            // Serialize the Message object to JSON
            string serializedMessage = JsonConvert.SerializeObject(message);

            // Write the serialized JSON to the memory-mapped stream
            Array.Copy(Encoding.ASCII.GetBytes(serializedMessage), buffer, Encoding.ASCII.GetBytes(serializedMessage).Length);
            mmvMemory.WriteArray(0, buffer, 0, nMaxSize);


            // Second - read the data from memory
            mmvMemory.ReadArray(0, buffer, 0, nMaxSize);
            string sJsonData = Encoding.ASCII.GetString(buffer, 0, nMaxSize);

            // Deserialize JSON data to a Message object
            Message receivedMessage = JsonConvert.DeserializeObject<Message>(sJsonData);
            //Console.WriteLine($"Client {nClientId} read from memory-mapped file: {receivedMessage.content}");

            // Clear the buffer
            Array.Clear(buffer, 0, nMaxSize);

            // Reset the synchronization value to release access
            unsafe
            {
                byte* ptr = null;

                // Get a pointer into the unmanaged memory of the view. 
                mmvSync.SafeMemoryMappedViewHandle.AcquirePointer(ref ptr);

                if (ptr != null) // Memory not accessible
                {
                    *(int*)ptr = 0;
                    mmvSync.SafeMemoryMappedViewHandle.ReleasePointer();
                }
            }
            nCallCount++;
        }
        //Thread.Sleep(1000);
    }
}
