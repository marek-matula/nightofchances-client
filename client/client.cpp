#include <iostream>
#include <windows.h> 
#include <vector>
#include <string>

#define BUFSIZE 512

class ClientConnection 
{
private:
    HANDLE m_Pipe = nullptr;
    std::wstring m_PipeName;
    DWORD m_PipeMode = PIPE_READMODE_MESSAGE;     // TODO remove
    std::vector<char> m_receiveBuffer;

public:
    ClientConnection(std::wstring pipeName = L"\\\\.\\pipe\\mynamedpipe"): m_PipeName(pipeName)
    {
        m_receiveBuffer.resize(BUFSIZE);
    };

    ~ClientConnection()
    {
        if(m_Pipe)
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
        if(!SetNamedPipeHandleState(
            m_Pipe,    // pipe handle 
            &m_PipeMode,  // new pipe mode 
            nullptr,     // don't set maximum bytes 
            nullptr)    // don't set maximum time
            )
        {
            std::cerr<<"SetNamedPipeHandleState failed. GLE= " << "\n";;
            return false;
        }

        return true;
    }

    bool Send(const std::vector<char>& buffer)
    {
        if (!m_Pipe)
            return false;

        DWORD cbWritten;
        if(!WriteFile(
            m_Pipe,                  // pipe handle 
            buffer.data(),             // message 
            (buffer.size()) * sizeof(char),// message length 
            &cbWritten,             // bytes written 
            nullptr)                  // not overlapped 
        )
        {
            std::cerr<< "WriteFile to pipe failed. GLE=%d" << "\n";
            return false;
        }
        return true;
    }

    std::vector<char> Receive()
    {
        if (!m_Pipe)
            return std::vector<char>();

        bool fSuccess = false;
        do
        {
            // Read from the pipe. 
            DWORD cbRead;
            fSuccess = ReadFile(
                m_Pipe,    // pipe handle 
                m_receiveBuffer.data(),    // buffer to receive reply 
                sizeof(char) * m_receiveBuffer.size(),  // size of buffer 
                &cbRead,  // number of bytes read 
                nullptr);    // not overlapped 

            if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
                break;

        } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

        if (!fSuccess)
        {
            std::cerr << "ReadFile from pipe failed. GLE=%" << "\n";
            return m_receiveBuffer;
        }
        return m_receiveBuffer;
    }

    std::vector<char> SendAndReceive(const std::vector<char>& buffer)
    {
        Send(buffer);
        return Receive();
    }
};

int main(int argc, TCHAR* argv[])
{
    ClientConnection connection;
    
    if (connection.ConnectToServer() == false)
        return 1;    

    std::string tmpStr = "Test message from client.";
    std::vector<char> inputBuffer(tmpStr.begin(), tmpStr.end());
    std::vector<char> reply = connection.SendAndReceive(inputBuffer);
    std::cout << reply.data() << "\n";

    tmpStr = "Second test message from client.";
    inputBuffer = std::vector<char>(tmpStr.begin(), tmpStr.end());
    reply = connection.SendAndReceive(inputBuffer);
    std::cout << reply.data() << "\n";

    return 0;
}