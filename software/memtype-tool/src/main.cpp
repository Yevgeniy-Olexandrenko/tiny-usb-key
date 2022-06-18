#include <iostream>
#include "memtype_api.h"

int main() 
{
	/* Just reads libusb version */
	memtype::Init();

	/* Open device USB port */
	if (memtype::Connect() == memtype::ret_t::Error)
	{
		return 1;
	};

	/* IsLocked example: */
	memtype::locked_t lock;

	if (memtype::IsLocked(&lock) == memtype::ret_t::NoError) 
	{
		printf("Device LOCK status: %s\n", (lock == memtype::locked_t::Locked) ? "LOCKED" : "NOT_LOCKED");
	}

	/* Read Info example: */
	memtype::info_t info;

	if (memtype::Info(&info) == memtype::ret_t::NoError) 
	{
		printf("Memtype Version: %03d.%03d.%03d CZ:%d\n", info.major, info.minor, info.patch, info.credSize);
	}
	else 
	{
		printf("Memtype_info err\n");
	}

	/* Keyboard example */
	uint8_t layout[128] =
	{ 
		0,   0,   0,   0,   0,   0,   0,   0,   42,  43,  40,  0,   0,   40,  0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		44,  158, 159, 96,  161, 162, 163, 45,  165, 166, 176, 48,  54,  56,  55,  164,
		39,  30,  31,  32,  33,  34,  35,  36,  37,  38,  183, 182, 3,   167, 131, 173,
		84,  132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
		147, 148, 149, 150,	151, 152, 153, 154, 155, 156, 157, 101, 109, 102, 175, 184,
		47,  4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,
		19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  100, 67,  103, 97,  0
	};
	if (memtype::WriteKeyboard(layout) == memtype::ret_t::NoError)
	{
		printf("Memtype Keyboard write\n");
	}
	else 
	{
		printf("Memtype_write_keyboard err\n");
	}

	/** Write Credential Example */
	uint16_t pin = 0;

	memtype::credential_t clist[] =
	{
		{"Area0x30", "usr", "\t", "pwd", "\n"},
		{"Area0x31", "usr", "\t", "pwd", "\n"},
		{"Area0x32", "usr", "\t", "pwd", "\n"},
		{"Area0x33", "usr", "\t", "pwd", "\n"},
		{"Area0x34", "usr", "\t", "pwd", "\n"},
	};
	uint16_t clen = sizeof(clist) / sizeof(memtype::credential_t);
	uint16_t buffsize = memtype::CredBuffSize(clist, sizeof(clist) / sizeof(memtype::credential_t));
	uint8_t* cred_buff = new uint8_t[buffsize];

	memtype::Encrypt(clist, clen, cred_buff, buffsize, pin);
	memtype::Write(cred_buff, buffsize, 0);
	delete[] cred_buff;

	/** Read Credentials Example */
	buffsize = 2048;
	cred_buff = new uint8_t[buffsize];
	memtype::Read(cred_buff, buffsize, 0);

	clen = memtype::CredLen(cred_buff, buffsize);
	memtype::credential_t* list = new memtype::credential_t[clen];
	memtype::Decrypt(list, clen, cred_buff, buffsize, pin);

	for (int i = 0; i < clen; i++) 
	{
		printf("name:%s user:%s pass:%s\n", list[i].name, list[i].user, list[i].pass);
	}
	delete[] list;
	delete[] cred_buff;

	/* Read/Write Pin example */
	uint8_t hash_w[16];
	uint8_t hash_r[16];

	memtype::PinToHash(pin, hash_w);
	memtype::WritePinHash(hash_w);
	memtype::ReadPinHash(hash_r);

	if (memcmp(hash_w, hash_r, 16) != 0) 
	{
		printf("Hash Write != Hash Read\n");
	}

	/* Disconnect example */
	memtype::Disconnect();

    return 0;
}