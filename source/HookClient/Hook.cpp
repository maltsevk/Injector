
#include <windows.h>
#include <fstream>
#include <string>
#include <tlhelp32.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

#ifdef _DEBUG
#include "Debug.h"
#endif

#include "Hook.h"

const HMODULE Hook::GetCurrentModule()
{
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;
	HMODULE hm = 0;
	::GetModuleHandleEx(flags, reinterpret_cast<LPCTSTR>(GetCurrentModule), &hm);
	return hm;
}

int Hook::ReplaceIATEntryInOneModule(PCSTR pszCalleeModName, PROC pfnCurrent, PROC pfnNew, HMODULE hmodCaller)
{
	// Getting pointer to array of import table
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = 
		(PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
			hmodCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize
		);
	if (pImportDesc == nullptr) {
		return -1;
	}

	// Searching for the desired module
	for (; pImportDesc->Name; pImportDesc++) {
		PSTR pszModName = (PSTR)((PBYTE)hmodCaller + pImportDesc->Name);
		if (lstrcmpiA(pszModName, pszCalleeModName) == 0) {
			break;
		}
	}
	if (pImportDesc->Name == 0) {
		return 0;
	}

	// Getting import table address
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hmodCaller + pImportDesc->FirstThunk);

	for (; pThunk->u1.Function; pThunk++) {
		if ((PROC)pThunk->u1.Function == pfnCurrent) {
			DWORD dwOldProtect;
			if (!VirtualProtect(&pThunk->u1.Function, sizeof(pThunk->u1.Function), PAGE_READWRITE, &dwOldProtect)) {
#ifdef _DEBUG
				Debug::log("        [-] VirtualProtect failed : " + std::to_string(GetLastError()));
#endif
				return -1;
			}

			// Changing from original function address to new address
			if (!WriteProcessMemory(GetCurrentProcess(), &pThunk->u1.Function, &pfnNew, sizeof(pfnNew), nullptr)) {
#ifdef _DEBUG
				Debug::log("        [-] WriteProcessMemory failed : " + std::to_string(GetLastError()));
#endif
				return -1;
			}

			VirtualProtect(&pThunk->u1.Function, sizeof(pThunk->u1.Function), dwOldProtect, &dwOldProtect);
			break;
		}
	}

	return 0;
}

void Hook::ReplaceIATEntryInAllModules(PCSTR pszCalleeModName, PROC pfnCurrent, PROC pfnNew)
{
	HMODULE hThisMod = GetCurrentModule();
	HANDLE hSnapshot = nullptr;
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

	// Includes all modules of the process specified in 
	// th32ProcessID in the snapshot
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (Module32First(hSnapshot, &me32))
	{
		do
		{
			if (hThisMod != me32.hModule)
			{
				int result = ReplaceIATEntryInOneModule(
					pszCalleeModName, 
					pfnCurrent, 
					pfnNew, 
					me32.hModule
				);
				if (result) {
#ifdef _DEBUG
					Debug::log("    [-] ReplaceIATEntryInOneModule failed");
#endif
				}
#ifdef _DEBUG
				Debug::log("    [+] Address of function successfully rewritten");
#endif
			}
		} while (Module32Next(hSnapshot, &me32));
	}

	CloseHandle(hSnapshot);
}