#pragma once

#include <stdint.h>

namespace noekeon
{
	void Encrypt(const uint32_t* const key, uint32_t* const data);
	void Decrypt(const uint32_t* const key, uint32_t* const data);
}
