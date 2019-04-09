#pragma once

#include <windows.h>

namespace Hook 
{
	int ReplaceIATEntryInOneModule(PCSTR pszCalleeModName, PROC pfnCurrent, PROC pfnNew, HMODULE hmodCaller);
	void ReplaceIATEntryInAllModules(PCSTR pszCalleeModName, PROC pfnCurrent, PROC pfnNew);
	const HMODULE GetCurrentModule();
}