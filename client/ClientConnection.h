#pragma once

#include <iostream>
#include <windows.h> 
#include <vector>
#include <string>

class ClientConnection
{
private:
    HANDLE m_Pipe = nullptr;
    std::wstring m_PipeName;
    DWORD m_PipeMode = PIPE_READMODE_MESSAGE;     // TODO remove
    std::vector<char> m_ReceiveBuffer;
    static const size_t m_BufSize = 512;

    ClientConnection(const ClientConnection&);
    void operator=(const ClientConnection&);

public:
    ClientConnection(std::wstring pipeName = L"\\\\.\\pipe\\mynamedpipe") : m_PipeName(pipeName)
    {
        m_ReceiveBuffer.resize(m_BufSize);
    };

    ~ClientConnection()
    {
        if (Connected())
            CloseHandle(m_Pipe);
    }

    bool ConnectToServer()
    {
        // Try to open a named pipe; wait for it, if necessary. 
        while (1)
        {
            m_Pipe = CreateFile(
                m_PipeName.c_str(),   // pipe name 
                GENERIC_READ |  // read and write access 
                GENERIC_WRITE,
                0,              // no sharing 
                nullptr,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe 
                0,              // default attributes 
                nullptr);          // no template file 

            // Break if the pipe handle is valid. 
            if (m_Pipe != INVALID_HANDLE_VALUE)
                break;

            // Exit if an error other than ERROR_PIPE_BUSY occurs. 

            if (GetLastError() != ERROR_PIPE_BUSY)
            {
                std::cerr << "Could not open pipe. GLE= " << GetLastError() << "\n";
                return false;
            }

            // All pipe instances are busy, so wait for 20 seconds. 

            if (!WaitNamedPipe(m_PipeName.c_str(), 20000))
            {
                std::cerr << "Could not open pipe: 20 second wait timed out." << "\n";
                return false;
            }
        }

        // The pipe connected; change to message-read mode. 
        if (!SetNamedPipeHandleState(
            m_Pipe,    // pipe handle 
            &m_PipeMode,  // new pipe mode 
            nullptr,     // don't set maximum bytes 
            nullptr)    // don't set maximum time
            )
        {
            std::cerr << "SetNamedPipeHandleState failed. GLE= " << "\n";;
            return false;
        }

        return true;
    }

    bool Connected()
    {
        return m_Pipe != nullptr;
    }

    bool Send(const std::vector<char>& buffer)
    {
        if (!m_Pipe)
            return false;

        DWORD writtenBytes;
        if (!WriteFile(
            m_Pipe,                  // pipe handle 
            buffer.data(),             // message 
            (DWORD)(buffer.size()) * sizeof(char),// message length 
            &writtenBytes,             // bytes written 
            nullptr)                  // not overlapped 
            )
        {
            std::cerr << "WriteFile to pipe failed. GLE=%d" << "\n";
            return false;
        }

        return true;
    }

    std::vector<char> Receive()
    {
        if (!m_Pipe)
            return std::vector<char>();

        bool success = false;
        do
        {
            // Read from the pipe. 
            DWORD readBytes;
            success = ReadFile(
                m_Pipe,    // pipe handle 
                m_ReceiveBuffer.data(),    // buffer to receive reply 
                (DWORD)m_ReceiveBuffer.size() * sizeof(char),  // size of buffer 
                &readBytes,  // number of bytes read 
                nullptr);    // not overlapped 

            if (!success && GetLastError() != ERROR_MORE_DATA)
                break;

        } while (!success);  // repeat loop if ERROR_MORE_DATA 

        if (!success)
        {
            std::cerr << "ReadFile from pipe failed. GLE=%" << "\n";
            return m_ReceiveBuffer;
        }
        return m_ReceiveBuffer;
    }

    std::vector<char> SendAndReceive(const std::vector<char>& buffer)
    {
        Send(buffer);
        return Receive();
    }
};