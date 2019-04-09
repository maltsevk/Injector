
#include <windows.h>
#include <iostream>
#include <string>

#include "MyNamedPipe.h"
#include "FunctionMonitor.h"
#include "FileHiding.h"

#ifdef _DEBUG
#include "Debug.h"
#endif

int initialize()
{
	int result = 0;
	std::string task;
	const std::string pipename = "\\\\.\\pipe\\injectornamedpipe";

	MyNamedPipe * pipe = new MyNamedPipe(pipename);

	// Opening the existing pipe and receiving task from injector
	try {
		pipe->openNamedPipe();

#ifdef _DEBUG
		Debug::log("[+] Pipe has been opened");
#endif

		while (true) {
			result = pipe->receiveMessage(task);
			if (result) {
				break;
			}

#ifdef _DEBUG
			Debug::log("[-] Task has not been received. Waiting for the next attempt...");
#endif

			Sleep(50);
		}
	}
	catch (std::string error) {
		Debug::log(std::move(error));
		return -1;
	}

#ifdef _DEBUG
	Debug::log("[+] Task from server has been received: " + task);
#endif

	std::string command = task.substr(0, 5);
	if (command == "-func") {
		std::string functionName = task.substr(6, task.length());
		FunctionMonitor::trackFunction(functionName, pipe);
		// Memory for pipe pointer will not be release
		// because it will be used by callback function
	}
	else if (command == "-hide") {
		std::string fileName = task.substr(6, task.length());
		FileHiding::hideFile(fileName);
		delete pipe;
	}
	else {
		return -1;
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, PVOID fImpLoad)
{

	switch (fdwReason) 
	{
		case DLL_PROCESS_ATTACH:
			if (initialize()) {
				return FALSE;
			}
			break;

		case DLL_PROCESS_DETACH:
#ifdef _DEBUG
			Debug::log("[*] DLL_PROCESS_DETACH");
#endif
			break;

		default:
			break;
	}

	return TRUE;
}