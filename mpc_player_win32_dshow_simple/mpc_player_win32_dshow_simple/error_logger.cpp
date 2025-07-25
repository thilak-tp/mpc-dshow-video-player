#include "error_logger.h"

//Used for error reporting
#include<comdef.h>

void ErrorLogger::Log(std::string msg) {
	std::string errorMsg = "Error: " + msg;
	MessageBoxA(NULL, errorMsg.c_str(), "Error", MB_ICONERROR);
}

void ErrorLogger::Log(HRESULT hr, std::string msg) {
	_com_error error(hr);
	std::wstring errorMsg = L"Error: " + StringConvertor::ToWString(msg) + L"\n" + std::wstring(error.ErrorMessage());
	MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_ICONERROR);
}