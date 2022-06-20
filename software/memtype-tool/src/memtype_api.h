#pragma once

#include <stdint.h>

namespace memtype
{
	enum class ret_t
	{
		NoError,
		Error
	};

	enum class locked_t
	{
		NotLocked,
		Locked
	};

	struct credential_t
	{
		const char* name;
		const char* user;
		const char* hop;
		const char* pass;
		const char* submit;
	};

	struct info_t
	{
		uint8_t  major;
		uint8_t  minor;
		uint8_t  patch;
		uint16_t credSize;
	};

	ret_t Init();
	ret_t Connect();
	ret_t Disconnect();

	ret_t Info(info_t* info);
	ret_t IsLocked(locked_t* lock);

	ret_t Write(const uint8_t* block, uint16_t len, uint16_t offset);
	ret_t Read(uint8_t* block, uint16_t len, uint16_t offset);

	ret_t WritePinHash(const uint8_t hash[16]);
	ret_t ReadPinHash(uint8_t hash[16]);
	ret_t WriteKeyboard(const uint8_t layout[128]);

	void PinToHash(uint16_t pin, uint8_t hash[16]);

	uint16_t CredBuffSize(credential_t* list, uint16_t len);
	uint16_t CredLen(uint8_t* buff, uint16_t size);

	ret_t Encrypt(credential_t* list, uint16_t len, uint8_t* buff, uint16_t size, uint16_t pin);
	ret_t Decrypt(credential_t* list, uint16_t len, uint8_t* buff, uint16_t size, uint16_t pin);
}