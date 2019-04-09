
#include <iostream>
#include <fstream>
#include <string>

#include "Debug.h"

void Debug::log(std::string && message)
{
	const char debugFileName[] = "debug.txt";

	message.append("\n");

	std::ofstream debugFile(debugFileName, std::ios_base::app);
	debugFile.write(message.c_str(), message.length());
	debugFile.close();
}

void Debug::log(std::wstring && wmessage)
{
	std::string message(wmessage.begin(), wmessage.end());
	Debug::log(std::move(message));
}

