
#pragma comment(lib, "advapi32.lib")

#include <aclapi.h>
#include <tchar.h>

#include "PipeServer.h"

int PipeServer::createSecurityAttributes(SECURITY_ATTRIBUTES * sa)
{
	DWORD dwRes;
	PSID pEveryoneSID = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

	// Create a well-known SID for the Everyone group
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
		SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pEveryoneSID)) {
		std::cout << "AllocateAndInitializeSid failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow Everyone full control to object
	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = GENERIC_ALL;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	// Create a new ACL that contains the new ACEs.
	dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes) {
		std::cout << "SetEntriesInAcl failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (NULL == pSD) {
		std::cout << "LocalAlloc failed : " << GetLastError() << std::endl;
		return -1;
	}

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
		std::cout << "InitializeSecurityDescriptor failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
		std::cout << "SetSecurityDescriptorDacl failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Initialize a security attributes structure.
	sa->nLength = sizeof(SECURITY_ATTRIBUTES);
	sa->lpSecurityDescriptor = pSD;
	sa->bInheritHandle = FALSE;

	return 0;
}

PipeServer::PipeServer(const std::string & pipename_)
{
	this->hPipe = nullptr;
	this->pipename = pipename_;
}

PipeServer::~PipeServer()
{
	CloseHandle(this->hPipe);
	this->hPipe = nullptr;
}

void PipeServer::createNamedPipe()
{
	const int bufsize = 512;

	SECURITY_ATTRIBUTES sa;
	if (this->createSecurityAttributes(&sa) < 0) {
		std::string error("[-] getSecurityDescriptor failed : " +
			std::to_string(GetLastError()));
		throw error;
	}

	this->hPipe = CreateNamedPipe(
		this->pipename.c_str(),   // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT |               // blocking mode 
		PIPE_ACCEPT_REMOTE_CLIENTS,
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		bufsize,                  // output buffer size 
		bufsize,                  // input buffer size 
		0,                        // client time-out 
		&sa);					  // security attribute 

	if (this->hPipe == INVALID_HANDLE_VALUE) {
		std::string error("[-] CreateNamedPipe failed : " + 
			std::to_string(GetLastError()));
		throw error;
	}
}

void PipeServer::waitForClient()
{
	BOOL fConnected = FALSE;

	while (true) {

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED
		fConnected = ConnectNamedPipe(this->hPipe, nullptr) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected) {
			return;
		}

		Sleep(300);
	}
}

int PipeServer::receiveMessage(std::string & message)
{
	BOOL fSuccess = FALSE;
	const int bufsize = 512;
	char buffer[bufsize];
	DWORD cbRead;

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

void PipeServer::sendMessage(const std::string & message)
{
	BOOL fSuccess = FALSE;
	DWORD cbWritten;

	fSuccess = WriteFile(
		this->hPipe,		// handle to pipe 
		message.c_str(),	// buffer to write from 
		message.length(),	// number of bytes to write 
		&cbWritten,			// number of bytes written 
		nullptr);			// not overlapped I/O 

	if (!fSuccess || message.length() != cbWritten) {
		std::string error("[-] WriteFile failed : " + 
			std::to_string(GetLastError()));
		throw error;
	}
}