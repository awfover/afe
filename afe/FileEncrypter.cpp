#include "stdafx.h"
#include "FileEncrypter.h"

bool FileEncrypter::Encrypt(const std::wstring &fout, const std::wstring &fin) {
	auto time_start = std::chrono::system_clock::now();

	std::ifstream is(fin, std::ifstream::binary);
	if (!is.is_open()) {
		SendMessage(_hwnd, Message::Type::Error, L"Cannot open file " + fin);
		return false;
	}

	std::wstring file_name;
	FileEncrypter::GetFileName(file_name, fin);

	std::wstring out_path;
	PathJoin(out_path, fout, file_name + L"." + FILE_ENCRYPTER_EXT);

	std::ofstream os(out_path, std::ofstream::binary);
	if (!os.is_open()) {
		SendMessage(_hwnd, Message::Type::Error, L"Cannot open file " + out_path);
		return false;
	}

	FileHeader fh;
	if (!WriteFileHeader(fh, os, is, file_name)) {
		SendMessage(_hwnd, Message::Type::Error, std::wstring(L"Write file header failed"));
		return false;
	}

	std::mutex mutex;
	size_t processed = 0;

	ThreadParam param = {
		&mutex,
		&is,
		&os,
		&processed,
		&fh,
		&_key,
		_hwnd,
	};

	bool final_result = true;

	std::vector<std::thread> threads;
	std::vector<std::future<bool>> results;
	for (int i = 0; i < FILE_ENCRYPTER_THREADS; i++) {
		std::packaged_task<bool(ThreadParam *)> task(static_cast<bool (*)(ThreadParam *)>(FileEncrypter::Encrypt));
		std::future<bool> result = task.get_future();

		std::thread thread(std::move(task), &param);
		threads.push_back(std::move(thread));
		results.push_back(std::move(result));
	}

	for (int i = 0; i < FILE_ENCRYPTER_THREADS; i++) {
		threads[i].join();
		final_result &= results[i].get();
	}

	if (!final_result) {
		SendMessage(_hwnd, Message::Type::Error, std::wstring(L"Encrypt failed"));
		return false;
	}

	auto time_end = std::chrono::system_clock::now();
	std::wstring duration;
	FormatDuration(duration, time_end - time_start);

	SendMessage(_hwnd, Message::Type::Success, L"Encrypt successed!\r\nOutput path: " + out_path + L"\r\nTime elapsed: " + duration);

	return true;
}

bool FileEncrypter::Encrypt(ThreadParam *param) {
	std::string in_buffer(FILE_ENCRYPTER_BUFFER_SIZE, 0);
	std::string out_buffer;

	aes::Aes e(param->key);
	uint64_t offset = 0;
	while (true) {
		{
			std::lock_guard<std::mutex> lock(*param->mutex);
			if (param->is->eof()) {
				return true;
			}

			offset = param->is->tellg();
			param->is->read(&in_buffer[0], FILE_ENCRYPTER_BUFFER_SIZE);
			in_buffer.resize((uint16_t)param->is->gcount(), 0);
		}

		if (!e.Encrypt(out_buffer, in_buffer)) {
			return false;
		}

		{
			std::lock_guard<std::mutex> lock(*param->mutex);
			param->os->seekp(param->fh->Size() + offset, std::ostream::beg);
			*(param->os) << out_buffer;

			*(param->processed) += in_buffer.size();
			SendMessage(param->hwnd, *(param->processed) * 1.0 / param->fh->file_size);
		}
	}
}

bool FileEncrypter::Decrypt(const std::wstring &fout, const std::wstring &fin) {
	auto time_start = std::chrono::system_clock::now();

	std::ifstream is(fin, std::ifstream::binary);
	if (!is.is_open()) {
		SendMessage(_hwnd, Message::Type::Error, L"Cannot open file " + fin);
		return false;
	}

	FileHeader fh;
	std::wstring file_name;
	if (!ReadFileHeader(fh, file_name, is)) {
		SendMessage(_hwnd, Message::Type::Error, std::wstring(L"Check file header failed"));
		return false;
	}

	std::wstring out_path;
	PathJoin(out_path, fout, file_name);

	std::ofstream os(out_path, std::ofstream::binary);
	if (!os.is_open()) {
		SendMessage(_hwnd, Message::Type::Error, L"Cannot open file " + out_path);
		return false;
	}

	std::mutex mutex;
	size_t processed = 0;

	ThreadParam param = {
		&mutex,
		&is,
		&os,
		&processed,
		&fh,
		&_key,
		_hwnd,
	};

	bool final_result = true;

	std::vector<std::thread> threads;
	std::vector<std::future<bool>> results;
	for (int i = 0; i < FILE_ENCRYPTER_THREADS; i++) {
		std::packaged_task<bool(ThreadParam *)> task(static_cast<bool(*)(ThreadParam *)>(FileEncrypter::Decrypt));
		std::future<bool> result = task.get_future();

		std::thread thread(std::move(task), &param);
		threads.push_back(std::move(thread));
		results.push_back(std::move(result));
	}

	for (int i = 0; i < FILE_ENCRYPTER_THREADS; i++) {
		threads[i].join();
		final_result &= results[i].get();
	}

	if (!final_result) {
		SendMessage(_hwnd, Message::Type::Error, std::wstring(L"Decrypt failed"));
		return false;
	}
	
	os.seekp(0, std::ofstream::end);
	if ((size_t)os.tellp() != fh.file_size) {
		SendMessage(_hwnd, Message::Type::Error, std::wstring(L"File size comparation failed"));
		return false;
	}

	auto time_end = std::chrono::system_clock::now();
	std::wstring duration;
	FormatDuration(duration, time_end - time_start);

	SendMessage(_hwnd, Message::Type::Success, L"Decrypt successed!\r\nOutput path: " + out_path + L"\r\nTime elapsed: " + duration);
	return true;
}

bool FileEncrypter::Decrypt(ThreadParam *param) {
	std::string in_buffer(FILE_ENCRYPTER_BUFFER_SIZE, 0);
	std::string out_buffer;

	aes::Aes e(param->key);
	uint64_t offset = 0;
	while (true) {
		{
			std::lock_guard<std::mutex> lock(*param->mutex);
			if (param->is->eof()) {
				return true;
			}

			offset = param->is->tellg();
			param->is->read(&in_buffer[0], FILE_ENCRYPTER_BUFFER_SIZE);
			in_buffer.resize((uint16_t)param->is->gcount(), 0);
		}

		if (!e.Decrypt(out_buffer, in_buffer)) {
			return false;
		}

		if (offset - param->fh->Size() + out_buffer.size() > param->fh->file_size) {
			out_buffer.resize((size_t)(param->fh->file_size - offset + param->fh->Size()));
		}

		{
			std::lock_guard<std::mutex> lock(*param->mutex);
			param->os->seekp(offset - param->fh->Size(), std::ostream::beg);
			*(param->os) << out_buffer;

			*(param->processed) += out_buffer.size();
			SendMessage(param->hwnd, *(param->processed) * 1.0 / param->fh->file_size);
		}
	}
}

bool FileEncrypter::ReadFileHeader(FileHeader &fh, std::wstring &file_name, std::ifstream &is) {
	is >> fh;
	if (fh.magic != FILE_ENCRYPTER_MAGIC) {
		return false;
	}

	aes::Aes e(&_key);
	std::string ansi_file_name;
	if (!e.Decrypt(ansi_file_name, fh.file_name)) {
		return false;
	}

	ansi_file_name.resize((size_t)fh.raw_file_name_size);
	util::AnsiToUnicode(file_name, ansi_file_name);

	return true;
}

bool FileEncrypter::WriteFileHeader(FileHeader &fh, std::ofstream &os, std::ifstream &is, const std::wstring &fin) {
	std::streampos p = is.tellg();
	is.seekg(0, std::ifstream::end);
	uint64_t file_size = is.tellg();
	is.seekg(p);

	std::string ansi_file_name;
	util::UnicodeToAnsi(ansi_file_name, fin);

	aes::Aes e(&_key);
	std::string sout;
	if (!e.Encrypt(sout, ansi_file_name)) {
		return false;
	}

	fh = FileHeader(FILE_ENCRYPTER_MAGIC, file_size, ansi_file_name.size(), sout);
	os << fh;

	return true;
}