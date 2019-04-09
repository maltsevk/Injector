#pragma once

#include <iostream>

class MyNamedPipe
{
private:
	HANDLE hPipe;
	std::string pipename;

public:
	explicit MyNamedPipe(const std::string &);
	~MyNamedPipe();

public:
	void openNamedPipe();
	void sendMessage(const std::string &);
	int receiveMessage(std::string &);
};