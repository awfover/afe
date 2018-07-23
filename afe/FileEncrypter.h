#pragma once

#include <mutex>
#include <chrono>
#include <vector>
#include <string>
#include <future>
#include <fstream>
#include <sstream>

#include "Aes.h"
#include "Util.h"

#define FILE_ENCRYPTER_MAGIC 0x4546410000000000
#define FILE_ENCRYPTER_EXT L"afe"
#define FILE_ENCRYPTER_THREADS 4
#define FILE_ENCRYPTER_BUFFER_SIZE 8192

#define WM_FILE_ENCRYPTER_MSG WM_USER + 888

class FileEncrypter {
public:
	struct Message {
		enum Type {
			Success,
			Progress,
			Error,
		};

		Type type;
		std::wstring msg;
	};

	struct ProgressMessage {
		Message::Type type;
		std::wstring msg;

		double progress;
	};

	struct FileHeader {
	public:
		uint64_t magic;
		uint64_t file_size;
		uint64_t raw_file_name_size;
		uint64_t enc_file_name_size;
		std::string file_name;

		FileHeader() = default;

		FileHeader(uint64_t magic, uint64_t file_size, uint64_t raw_file_name_size, const std::string &file_name) :
			magic(magic),
			file_size(file_size),
			raw_file_name_size(raw_file_name_size),
			enc_file_name_size(file_name.size()),
			file_name(file_name) {}

		~FileHeader() {}

		friend std::ostream & operator << (std::ostream &os, const FileHeader &fh) {
			os.write((char *)&fh.magic, sizeof(uint64_t));
			os.write((char *)&fh.file_size, sizeof(uint64_t));
			os.write((char *)&fh.raw_file_name_size, sizeof(uint64_t));
			os.write((char *)&fh.enc_file_name_size, sizeof(uint64_t));
			os << fh.file_name;
			return os;
		}

		friend std::istream & operator >> (std::istream &is, FileHeader &fh) {
			is.read((char *)&fh.magic, sizeof(uint64_t));
			is.read((char *)&fh.file_size, sizeof(uint64_t));
			is.read((char *)&fh.raw_file_name_size, sizeof(uint64_t));
			is.read((char *)&fh.enc_file_name_size, sizeof(uint64_t));

			fh.file_name.resize((size_t)fh.enc_file_name_size, 0);
			is.read(&fh.file_name[0], fh.file_name.size());

			return is;
		}

		size_t Size() const {
			return (sizeof(uint64_t) * 4 + file_name.size());
		}
	};

	struct ThreadParam {
		std::mutex *mutex;
		std::istream *is;
		std::ostream *os;
		size_t *processed;
		FileHeader *fh;
		aes::Key *key;
		HWND hwnd;
	};

	FileEncrypter() = delete;
	FileEncrypter(const HWND &hwnd, const std::wstring &key)
		: _hwnd(hwnd), _key(key) {}
	virtual ~FileEncrypter() {}

	bool Encrypt(const std::wstring &fout, const std::wstring &fin);
	static bool Encrypt(ThreadParam *param);

	bool Decrypt(const std::wstring &fout, const std::wstring &fin);
	static bool Decrypt(ThreadParam *param);

private:
	aes::Key _key;
	HWND _hwnd;

	bool ReadFileHeader(FileHeader &fh, std::wstring &file_name, std::ifstream &is);
	bool WriteFileHeader(FileHeader &fh, std::ofstream &os, std::ifstream &is, const std::wstring &fin);

	static void GetFileName(std::wstring &out, const std::wstring &in) {
		size_t p = in.find_last_of('\\');
		if (p == std::wstring::npos) {
			out = in;
		}
		else {
			out = in.substr(p + 1);
		}
	}

	static void PathJoin(std::wstring &out, const std::wstring &path) {
		if (path.size() == 0) {
			return;
		}
		if ((out.size() > 0) && (out.back() != '\\')) {
			out += '\\';
		}
		out += path;
	}

	template <class ...Args>
	static void PathJoin(std::wstring &out, const std::wstring &path, Args&&... args) {
		PathJoin(out, path);
		PathJoin(out, std::forward<Args>(args)...);
	}
	
	static void FormatDuration(std::wstring &out, std::chrono::duration<double> duration) {
		uint64_t hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
		if (hours > 24) {
			out = L"more than 1 day";
			return;
		}
		if (hours >= 1) {
			out += std::to_wstring(hours) + L"h ";
			duration -= std::chrono::hours(hours);
		}

		uint64_t minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count();
		if (minutes >= 1) {
			out += std::to_wstring(minutes) + L"m ";
			duration -= std::chrono::minutes(minutes);
		}

		uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		if (seconds >= 1) {
			out += std::to_wstring(seconds) + L"s ";
		}

		if (out.size() > 0) {
			return;
		}

		uint64_t milli = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		out = std::to_wstring(milli) + L"ms";
	}

	static void SendMessage(HWND hwnd, Message::Type type, std::wstring &msg) {
		Message m = {
			Message::Type::Error,
			msg,
		};

		::SendMessageA(hwnd, WM_FILE_ENCRYPTER_MSG, 0, (LPARAM)&m);
	}

	static void SendMessage(HWND hwnd, double progress) {
		ProgressMessage m = {
			Message::Type::Progress,
			L"",
			progress,
		};

		::SendMessageA(hwnd, WM_FILE_ENCRYPTER_MSG, 0, (LPARAM)&m);
	}
};