#define _CRT_SECURE_NO_WARNINGS

#include <ctime>
#include <string>
#include <windows.h>
#include <iostream>

#include "Hook.h"

#ifdef _DEBUG
#include "Debug.h"
#endif

#include "FunctionMonitor.h"

std::string monitoredFunctionName;
FARPROC pMonitoredFunction;
MyNamedPipe * pipe;

char * getCurrentTime()
{
	time_t seconds = time(nullptr);
	tm* timeinfo = localtime(&seconds);
	return asctime(timeinfo);
}

void sendMessageAboutFunctionCall()
{
	std::string message(getCurrentTime());

	message.append(" ");
	message.append(::monitoredFunctionName);
	::pipe->sendMessage(message);
}

LPVOID __declspec(naked) Hook_TrackingFunction()
{
	// Now stack has agruments for pMonitoredFunction
	__asm {
		call sendMessageAboutFunctionCall
		jmp pMonitoredFunction
	}
}

int FunctionMonitor::trackFunction(std::string & functionName, MyNamedPipe * pipe)
{
	::pipe = pipe;
	::monitoredFunctionName = functionName;

	::pMonitoredFunction = GetProcAddress(
		GetModuleHandle("KERNEL32.dll"), 
		::monitoredFunctionName.c_str()
	);
	if (::pMonitoredFunction == nullptr) {
#ifdef _DEBUG
		Debug::log("[-] GetProcAddress : " + std::to_string(GetLastError()));
#endif
		return -1;
	}

	Hook::ReplaceIATEntryInAllModules(
		"kernel32.dll",
		::pMonitoredFunction,
		(PROC)Hook_TrackingFunction
	);

	return 0;
}