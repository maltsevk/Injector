
#include <string>
#include <windows.h>

#include "MyNamedPipe.h"

MyNamedPipe::MyNamedPipe(const std::string & pipename_)
{
	this->hPipe = nullptr;
	this->pipename = pipename_;
}

MyNamedPipe::~MyNamedPipe()
{
	CloseHandle(this->hPipe);
	this->hPipe = nullptr;
}

void MyNamedPipe::openNamedPipe()
{
	DWORD dwMode;
	BOOL fSuccess = FALSE;

	// Try to open a named pipe; wait for it, if necessary
	while (true) {
		this->hPipe = CreateFile(
			this->pipename.c_str(),	// pipe name 
			GENERIC_READ |		// read and write access 
			GENERIC_WRITE,
			0,					// no sharing 
			NULL,				// default security attributes
			OPEN_EXISTING,		// opens existing pipe 
			0,					// default attributes 
			NULL);				// no template file 

		// Break if the pipe handle is valid
		if (this->hPipe != INVALID_HANDLE_VALUE) {
			break;
		}

		// Exit if an error other than ERROR_PIPE_BUSY occurs
		if (GetLastError() != ERROR_PIPE_BUSY) {
			std::string error("[-] CreateFileA failed : " +
				std::to_string(GetLastError()));
			throw error;
		}

		// All pipe instances are busy, so wait for 2 seconds 
		if (!WaitNamedPipe(pipename.c_str(), 1000)) {
			std::string error("[-] WaitNamedPipeA failed : " +
				std::to_string(GetLastError()));
			throw error;
		}

		Sleep(100);
	}

	// The pipe connected; change to message-read mode
	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(this->hPipe, &dwMode, NULL, NULL);
	if (!fSuccess) {
		std::string error("[-] SetNamedPipeHandleState failed : " +
			std::to_string(GetLastError()));
		throw error;
	}
}

int MyNamedPipe::receiveMessage(std::string & message)
{
	BOOL fSuccess = FALSE;
	const int bufsize = 512;
	char buffer[bufsize];
	DWORD cbRead = 0;

	// Read from the pipe
	fSuccess = ReadFile(
		this->hPipe,	// pipe handle 
		buffer,			// buffer to receive reply 
		bufsize,		// size of buffer 
		&cbRead,		// number of bytes read 
		nullptr);		// not overlapped 

	if (!fSuccess) {
		std::string error("[-] ReadFile failed : " +
			std::to_string(GetLastError()));
		throw error;
	}

	if (cbRead != 0) {
		message.append(buffer, cbRead);
	}

	return cbRead;
}

void MyNamedPipe::sendMessage(const std::string & message)
{
	BOOL fSuccess = FALSE;
	DWORD cbWritten;

	fSuccess = WriteFile(
		this->hPipe,		// pipe handle 
		message.c_str(),    // message 
		message.length(),   // message length 
		&cbWritten,         // bytes written 
		NULL);              // not overlapped 

	if (!fSuccess) {
		std::string error("[-] WriteFile failed : " +
			std::to_string(GetLastError()));
		throw error;
	}
}