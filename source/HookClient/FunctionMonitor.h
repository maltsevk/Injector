#pragma once

#include "MyNamedPipe.h"

namespace FunctionMonitor
{
	int trackFunction(std::string &, MyNamedPipe * pipe);
}