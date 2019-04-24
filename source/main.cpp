
#include "PipeServer.h"

#include <iostream>
#include <string>

#include <windows.h>
#include <tlhelp32.h>



HANDLE getProcessHandleByName(char * processName)
{
	HANDLE hProcess = nullptr;
	HANDLE hSnapshot = nullptr;
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

	// Takes a snapshot of the all processes in the system
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		std::cout << "[-] CreateToolhelp32Snapshot failed: " << GetLastError() << std::endl;
		return nullptr;
	}

	// Retrieve information about the first process
	if (!Process32First(hSnapshot, &pe32)) {
		std::cout << "[-] Process32First failed: " << GetLastError() << std::endl;
		CloseHandle(hSnapshot);
		return nullptr;
	}

	// Now walk the snapshot of processes
	do {
		if (std::string(pe32.szExeFile) == std::string(processName)) {
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if (hProcess == nullptr) {
				std::cout << "[-] OpenProcess failed: " << GetLastError() << std::endl;
				CloseHandle(hSnapshot);
				return nullptr;
			}
				
			CloseHandle(hSnapshot);
			return hProcess;
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return nullptr;
}

HANDLE getProcessHandle(char * argumentSpecifier, char * processNameOrId)
{
	HANDLE hProcess = nullptr;

	if (std::string(argumentSpecifier) == "-pid") {
		DWORD targetProcessId = std::stoi(processNameOrId);

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
		if (!hProcess) {
			std::cout << "[-] OpenProcess failed : " << GetLastError() << std::endl;
		}
	}
	else if (std::string(argumentSpecifier) == "-name") {
		hProcess = getProcessHandleByName(processNameOrId);
	}
	else {
		std::cout << "[-] Invalid argument specifier" << std::endl;
	}

	return hProcess;
}

int injectDllToTheProcess(HANDLE hProcess, const std::string & dllPath)
{
	LPVOID lpfnLoadLibraryA;
	LPVOID lpvResult;
	HANDLE hThread;

	// Getting function to load DLL to the process space
	lpfnLoadLibraryA = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (!lpfnLoadLibraryA) {
		std::cout << "[-] GetProcAddress failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Allocating of the memory area for the subsequent writing 
	// of the library name 
	lpvResult = VirtualAllocEx(hProcess, nullptr, dllPath.size(), MEM_COMMIT, PAGE_READWRITE);
	if (!lpvResult) {
		std::cout << "[-] VirtualAllocEx failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Writing of the injecting DLL name to the memory
	if (!WriteProcessMemory(hProcess, lpvResult, dllPath.c_str(), dllPath.size(), nullptr)) {
		std::cout << "[-] WriteProcessMemory failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Creating a remote thread in the address space of an open
	// process and then DLL loading
	hThread = CreateRemoteThread(hProcess, nullptr, 0, 
		(LPTHREAD_START_ROUTINE)lpfnLoadLibraryA, lpvResult, 0, nullptr);
	if (!hThread) {
		std::cout << "[-] CreateRemoteThread failed : " << GetLastError() << std::endl;
		return -1;
	}

	CloseHandle(hThread);

	return 0;
}

int main(int argc, char * argv[])
{
	if (argc != 5) {
		std::cout << "[-] Invalid number of arguments" << std::endl;
		return -1;
	}

	HANDLE hProcess;
	const std::string pipename = "\\\\.\\pipe\\injectornamedpipe";
	const std::string task(std::string(argv[3]) + 
		std::string(" ") + std::string(argv[4]));

	char currentDirectory[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(currentDirectory), currentDirectory);

	std::string dllPath(currentDirectory);
	dllPath.append("\\HookClient.dll");

	std::string message;
	int result = 0;

	// Getting process handle by name or pid
	hProcess = getProcessHandle(argv[1], argv[2]);
	if (!hProcess) {
		return -1;
	}

	std::cout << "[+] Process handle has been got" << std::endl;

	// Creating a named pipe before injecting
	PipeServer pipe(pipename);
	try {
		pipe.createNamedPipe();
	}
	catch (std::string & error) {
		std::cout << error << std::endl;
		return -1;
	}

	std::cout << "[+] Pipe object has been created" << std::endl;

	// Injecting DLL to the process
	if (injectDllToTheProcess(hProcess, dllPath)) {
		return -1;
	}

	std::cout << "[+] " << dllPath << " has been injected to " 
		<< argv[2] << std::endl;

	std::cout << "[*] Waiting for a pipe client... " << std::endl;

	// Waiting for the client to connect
	pipe.waitForClient();

	std::cout << "[+] Pipe client has been connected" << std::endl;

	// Sending a task to hook client
	try {
		pipe.sendMessage(task);
	}
	catch (std::string & error) {
		std::cout << error << std::endl;
		return -1;
	}

	std::cout << "[+] Task to client has been sent" << std::endl;

	if (std::string("-func") == argv[3]) {
		std::cout << "[*] Waiting messages from client... " << std::endl;

		while (true) {

			try {
				result = pipe.receiveMessage(message);
			}
			catch (std::string & error) {
				std::cout << error << std::endl;
				return -1;
			}

			if (result) {
				std::cout << message << std::endl;
				message.clear();
			}

			Sleep(100);
		}
	}

	std::cout << "[+] Injector has finished" << std::endl;

	return 0;
}