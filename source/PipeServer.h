#pragma once

#include <windows.h>
#include <iostream>
#include <string>

class PipeServer
{
private:
	HANDLE hPipe;
	std::string pipename;

public:
	explicit PipeServer(const std::string &);
	~PipeServer();

public:
	void createNamedPipe();
	void waitForClient();
	void sendMessage(const std::string &);
	int receiveMessage(std::string &);

private:
	int createSecurityAttributes(SECURITY_ATTRIBUTES *);
};