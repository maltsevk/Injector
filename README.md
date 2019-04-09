# Description
This project monitors the call functions in the process. The program uses the Hook & Inject technique, which is to force the target process to load the specified DLL, which, in turn, modifies the tables with DLL functions, intercepting them. When using such a technique, it is possible to monitor the call of specified functions, and it is also possible to replace the values of the result of the function by changing the behavior of the target process.

### Requirements
- Windows x32
- Administrative rights for injector launch

### Usage

Launch injector for function monitoring
```
Injector.exe –pid <pid> –func <function_name>
```
Launch injector for function monitoring
```
Injector.exe –name <process_name> –hide <filename>
```
