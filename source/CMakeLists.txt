cmake_minimum_required(VERSION 3.0)	# Проверка версии CMake.
									# Если версия установленой программы
									# старее указаной, произайдёт аварийный выход.

project(Injector)	# Название проекта

set(SOURCE_EXE main.cpp PipeServer.cpp)		# Установка переменной со списком исходников

include_directories(${CMAKE_CURRENT_SOURCE_DIR})			# Расположение заголовочных файлов - текущая папка

add_executable(Injector ${SOURCE_EXE})	# Создает исполняемый файл с именем Injector

add_subdirectory(HookClient)				# Добавление подпроекта, указывается имя дирректории

