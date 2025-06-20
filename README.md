# Inter & Intra-Process Communication Examples

This is a **personal repository** where I keep track of various examples and experiments involving **inter-process** and **intra-process communication (IPC)** across different programming languages.

The goal is to better understand system-level messaging, shared memory models, synchronization primitives, and how different platforms and languages handle communication between threads and processes.

## Repository Structure

### `Inter_Process_Com/`

This folder contains a **classic client-server IPC setup**, primarily in **C++ on Windows**, using:

* **Named events** (`CreateEvent`, `WaitForSingleObject`, etc.)
* **Shared memory** (`CreateFileMapping`, `MapViewOfFile`)

This setup demonstrates:

* Synchronized message-passing between a client and a server
* Event-based signaling
* Memory mapping for data exchange

Useful for understanding **Windows-specific IPC** with low-level synchronization and resource cleanup logic.

### `Inter_Process_Com_Shared_Memory_No_Server/`

This folder explores IPC **without a traditional server process**, using **shared memory** directly across:

* Pure **C++**
* **C++/CLI** (Managed C++ for .NET interop)
* **C#**

These examples illustrate:

* Memory-mapped communication across different languages within the Windows environment
* Mixed managed/unmanaged code integration via `.NET`
* Basic use of atomic operations and interlocked synchronization

### `Intra_Process_Com/`

This folder contains examples of **communication within the same process**, i.e., between threads, using:

* **C++**
* **C++/CLI**
* **Visual Basic (VB.NET)**

Mechanisms used include:

* Shared variables
* AutoResetEvent-style synchronization
* Timers and counters
* Thread-safe atomic operations (`Interlocked`)

These examples are simple but illustrative of **thread coordination and race condition prevention**.



This is **not meant to be a polished or production-ready library**. However, if you're exploring **low-level systems programming**, Windows IPC, or **cross-language runtime communication**, you may find some examples here useful or adaptable.

## Notes

* All examples are written and tested on **Windows OS**.
* Most of the IPC mechanisms here use **Win32 API** or rely on .NET threading primitives.


## Future Additions

I may add:

* Named pipes / sockets examples
* POSIX shared memory or Linux IPC equivalents
* Cross-machine messaging (RPC, gRPC, etc.)
* Benchmarking or visualization tools for IPC performance