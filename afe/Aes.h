#pragma once

#include <string>
#include <vector>

#include <fstream>

#include "Util.h"

namespace aes {

	namespace constant {
		static const size_t AES_NR = 10;
		static const size_t AES_NB = 4;
		static const size_t AES_NK = 4;

		extern const uint8_t SBOX[];
		extern const uint8_t INV_SBOX[];
		extern const uint8_t XTIME[];
		extern const uint32_t RCON[];
	}

	namespace util {
		uint32_t SubDword(uint32_t x);
		uint32_t RotDword(uint32_t x);
		size_t CalcSize(size_t s);
	}

	class Key {
	public:
		Key() = delete;
		explicit Key(const std::wstring &cipher);

		uint32_t operator [] (size_t i) const {
			return _round_keys[i];
		}

	private:
		uint8_t _key[4 * constant::AES_NK];
		uint8_t _round_keys[4 * constant::AES_NK * (constant::AES_NR + 1)];

		void KeyExpansion();
	};

	class Aes {
	public:
		Aes() = delete;
		explicit Aes(const Key *key)
			: _key(key) {}

		virtual ~Aes() {}

		bool Encrypt(std::string &sout, const std::string &sin);
		bool Encrypt(std::vector<uint8_t> &dout, const std::string &sin);
		bool Encrypt(uint8_t *dout, const uint8_t *din);

		bool Decrypt(std::string &sout, const std::string &sin);
		bool Decrypt(std::vector<uint8_t> &dout, const std::string &sin);
		bool Decrypt(uint8_t *dout, const uint8_t *din);

	private:
		const Key *_key;
		uint8_t _state[4 * constant::AES_NB];

		void SubBytes();
		void ShiftRows();
		void MixColumns();

		void InvSubBytes();
		void InvShiftRows();
		void InvMixColumns();

		void AddRoundKey(unsigned int round);
	};

}