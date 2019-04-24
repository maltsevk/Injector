cmake_minimum_required(VERSION 3.0) # Если версия меньше, то ошибка

project(HookClient)				# Название проекта

set(SOURCE_DLL Debug.cpp FileHiding.cpp FunctionMonitor.cpp Hook.cpp main.cpp MyNamedPipe.cpp)		# Установка переменной со списком исходников
include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # Расположение заголовочных файлов - текущая папка
add_library(HookClient SHARED ${SOURCE_DLL})# Создание динамической библиотеки библиотеки