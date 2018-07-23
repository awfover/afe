#include "stdafx.h"

#include "Util.h"

namespace util {
	
	bool AnsiToUnicode(std::wstring &out, const std::string &in) {
		size_t size = in.size();
		int wsize = MultiByteToWideChar(CP_UTF8, 0, &in[0], size, nullptr, 0);

		out.resize(wsize, 0);
		int ret = MultiByteToWideChar(CP_UTF8, 0, &in[0], size, &out[0], wsize);

		return (ret > 0);
	}

	bool UnicodeToAnsi(std::string &out, const std::wstring &in) {
		size_t wsize = in.size();
		int size = WideCharToMultiByte(CP_UTF8, 0, &in[0], wsize, nullptr, 0, nullptr, FALSE);

		out.resize(size, 0);
		int ret = WideCharToMultiByte(CP_UTF8, 0, &in[0], wsize, &out[0], size, nullptr, FALSE);

		return (ret > 0);
	}

}