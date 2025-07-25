#include "string_convertor.h"

std::wstring StringConvertor::ToWString(const std::string& str)
{
	std::wstring wideString(str.begin(), str.end());
	return wideString;
}

std::string StringConvertor::ToString(const std::wstring& wstr)
{
	std::string narrowString(wstr.begin(), wstr.end());
	return narrowString;
}