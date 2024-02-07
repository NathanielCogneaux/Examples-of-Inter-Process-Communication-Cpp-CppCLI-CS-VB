Imports System.Threading

Module TrivialIntraprocessComusingVB

    Public nCounter As Long
    Public nShareMemory As Long

    ' Signaled event: the wicket is initialy open/signaled
    Public eClientWicket As New AutoResetEvent(True)

    ' Unsignaled event the wicket is initialy closed/unsignaled
    Public eServerWicket As New AutoResetEvent(False)

    Public lWeOff As Boolean

    Sub Main()

        ' Timer launch
        Dim tState = New Timer(AddressOf State, Nothing, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1))

        ' Client (writer / applicant) thread launch
        Dim tClient As New Thread(AddressOf Client)
        tClient.Start()

        ' Server (reader / counter clerk) thread launch
        Dim tServer As New Thread(AddressOf Server)
        tServer.Start()

        ' Wait for Enter
        Console.ReadLine()

        lWeOff = True

        Console.WriteLine("Main exit")
    End Sub

    Sub Client()

        Do
            ' The wicket is initially open (signaled) and will close automatically after passing through (AutoResetevent)
            eClientWicket.WaitOne()

            ' New application data...
            nShareMemory += 1

            ' Open the server wicket
            eServerWicket.Set()

        Loop Until lWeOff

        Console.WriteLine("Client thread exit")
    End Sub
    Sub Server()

        Do
            ' The wicket is initialy closed (unsignaled) and will automatically close after passing through (AutoReset event)
            eServerWicket.WaitOne()

            ' Some important job...
            If nShareMemory = 100000 Then
                nShareMemory = 0
            End If

            ' Service counter (possible concurrency with State)
            Interlocked.Increment(nCounter)

            ' Open the client wicket
            eClientWicket.Set()

        Loop Until lWeOff

        Console.WriteLine("Server thread exit")
    End Sub

    Sub State()

        Console.WriteLine($"Call per s.: {nCounter}")

        ' (possible concurrency with Server)
        Interlocked.CompareExchange(nCounter, 0, nCounter)
    End Sub
End Module
