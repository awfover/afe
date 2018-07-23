#pragma once

#include <string>

namespace util {
	
	bool AnsiToUnicode(std::wstring &out, const std::string &in);
	bool UnicodeToAnsi(std::string &out, const std::wstring &in);

}