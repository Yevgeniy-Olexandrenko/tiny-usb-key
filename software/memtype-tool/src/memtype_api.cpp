#include "libusb/libusb.h"
#include "memtype_api.h"
#include "noekeon_api.h"
#include <cstdio>

/* Sleep Time includes */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

/* CONFIGURATION defines */
#define USB_WAIT_10ms() { Sleep(10); }
#define USB_WAIT_50ms() { Sleep(50); }

namespace memtype
{
	/* Local Vars */
	static uint16_t vendor_id  = 0x1209;
	static uint16_t product_id = 0xA033;
	static libusb_device_handle* dev;

	enum
	{
		CMD_INFO           = 0x05,
		CMD_WRITE_BLOCK    = 0x03,
		CMD_READ_BLOCK     = 0x02,
		CMD_WRITE_PIN_HASH = 0x07,
		CMD_READ_PIN_HASH  = 0x08,
		CMD_WRITE_KEYBOARD = 0x09,
	};

	static void PinToKey(const uint8_t pin[4], uint32_t key[4])
	{
		memcpy(&key[0], pin, 4);
		memcpy(&key[1], pin, 4);
		memcpy(&key[2], pin, 4);
		memcpy(&key[3], pin, 4);
	};

	static uint16_t EncryptedSize(const credential_t* cred)
	{
		size_t len = 4;
		len += strlen(cred->user);
		len += strlen(cred->hop);
		len += strlen(cred->pass);
		len += strlen(cred->submit);

		/* len must be multiple of 16 bytes */
		if ((len % 16) != 0) 
		{
			/* add padding */
			len += 16 - (len % 16);
		}
		return (uint16_t)len;
	}

	ret_t Receive(uint8_t* msg, uint8_t* len)
	{
		ret_t ret = ret_t::NoError;
		uint16_t reportId = 2;

		/** Usb HID Get Report */
		/** dev_handle, bmRequestType, bmRequest, wValue, wIndex, data, wLength, timeout */
		if (libusb_control_transfer(dev, 0xA0, 0x01, 0x0300 | reportId, 0, msg, *len, 5000) != *len)
		{
			printf("Error USB HID get report\n");
		}

		/* Active wait to let memtype do his stuff */
		USB_WAIT_10ms();
		return ret;
	}

	ret_t Send(uint8_t* msg, uint8_t* len)
	{
		ret_t ret = ret_t::NoError;
		uint16_t reportId = 2;

		/** Usb HID Set Report */
		/** dev_handle, bmRequestType, bmRequest, wValue, wIndex, data, wLength, timeout */
		if (libusb_control_transfer(dev, 0x20, 0x09, 0x0300 | reportId, 0, msg, *len, 5000) != *len)
		{
			printf("Error USB HID set report\n");
			ret = ret_t::Error;
		}

		/* Active wait to let memtype do his stuff */
		USB_WAIT_10ms();
		return Receive(msg, len);
	}

	ret_t Init()
	{
		const struct libusb_version* version = libusb_get_version();

		/* Print LibUSB version */
		printf("LibUSB version -- %d.%d.%d.%d %s\n",
			version->major, version->minor, version->micro, version->nano, version->rc);

		return ret_t::NoError;
	}

	ret_t Connect()
	{
		static bool usbInit = false;
		
		/* Init libusb */
		if (!usbInit) 
		{
			libusb_init(NULL);
			usbInit = true;
		}

		/* Open device directly */
		dev = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);

		/* libusb_claim_interface ??? */
		if (dev == NULL) 
		{
			printf("Could not open Memtype\n");
			return ret_t::Error;
		}

		return ret_t::NoError;
	}

	ret_t Disconnect()
	{
		/* Close USB connection */
		if (dev != NULL) 
		{
			libusb_close(dev);
			return ret_t::NoError;
		}
		else 
		{
			printf("Could not close Memtype\n");
			return ret_t::Error;
		}
	}

	ret_t Info(info_t* info)
	{
		uint8_t msg[8] = { CMD_INFO, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t len = sizeof(msg);

		/* Send CMD */
		ret_t ret = Send(msg, &len);

		if (info && msg[0] == CMD_INFO)
		{
			info->major = msg[1];
			info->minor = msg[2];
			info->patch = msg[3];
			info->credSize = (msg[4] | msg[5] << 8);
		}
		else
		{
			ret = ret_t::Error;
		}
		return ret;
	}

	// TODO: combine with Info command?
	ret_t IsLocked(locked_t* lock)
	{
		uint8_t msg[8] = { CMD_INFO, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t len = sizeof(msg);

		/* Send CMD */
		ret_t ret = Send(msg, &len);

		if (lock) // TODO:: check for command?
		{
			*lock = (msg[1] == 0xF6 ? locked_t::Locked : locked_t::NotLocked);
		}
		else 
		{
			ret = ret_t::Error;
		}
		return ret;
	}

	ret_t Write(const uint8_t* block, uint16_t len, uint16_t offset)
	{
		ret_t ret;
		uint8_t* buff;

		uint8_t msg[8] = { CMD_WRITE_BLOCK, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t l = sizeof(msg);

		/* Prepare command for offset and size */
		msg[1] = uint8_t(offset);
		msg[2] = uint8_t(offset >> 8);
		msg[3] = uint8_t(len);
		msg[4] = uint8_t(len >> 8);

		/* Prepare Buffer to dynamic allocated memory */
		/* Allocate 8 bytes more than needed */
		//buff = (uint8_t*)malloc(len + 8);
		buff = new uint8_t[len + 8];

		if (block && buff && (offset + len) <= 2048) 
		{
			/* Send CMD */
			ret = Send(msg, &l);
			memcpy(buff, block, len);

			if (msg[0] == CMD_WRITE_BLOCK)
			{
				/* Send Data to write */
				for (uint16_t i = 0; i < len; i += 8) 
				{
					l = 8;
					Send(&buff[i], &l);
					USB_WAIT_50ms();
				}

				/* Compare buff and block to be equal */
				if (memcmp(buff, block, len) != 0) 
				{
					ret = ret_t::Error;
				}
			}
			else 
			{
				ret = ret_t::Error;
			}
		}
		else 
		{
			ret = ret_t::Error;
		}

		/* Free dynamic allocated memory */
		//free(buff);
		delete[] buff;

		return ret;
	}

	ret_t Read(uint8_t* block, uint16_t len, uint16_t offset)
	{
		ret_t ret;
		uint8_t* buff = block;

		uint8_t msg[8] = { CMD_READ_BLOCK, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t l = sizeof(msg);
		
		/* Prepare command for offset and size */
		msg[1] = uint8_t(offset);
		msg[2] = uint8_t(offset >> 8);
		msg[3] = uint8_t(len);
		msg[4] = uint8_t(len >> 8);

		if (block && (offset + len) <= 2048 && (len % 8) == 0) 
		{
			/* Send CMD */
			ret = Send(msg, &l);

			if (msg[0] == CMD_READ_BLOCK)
			{
				/* Read Data */
				for (uint16_t i = 0; i < len; i += 8) 
				{
					l = 8;
					Receive(&buff[i], &l);
				}
			}
			else 
			{
				ret = ret_t::Error;
			}
		}
		else 
		{
			ret = ret_t::Error;
		}
		return ret;
	}

	ret_t WritePinHash(const uint8_t hash[16])
	{
		ret_t ret;
		uint8_t buff[16];

		uint8_t msg[8] = { CMD_WRITE_PIN_HASH, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t len = sizeof(msg);

		if (hash)
		{
			/* Send CMD */
			ret = Send(msg, &len);
			memcpy(buff, hash, sizeof(buff));

			if (msg[0] == CMD_WRITE_PIN_HASH)
			{
				/* Send Pin Hash */
				for (uint8_t i = 0; i < sizeof(buff); i += 8) 
				{
					len = 8;
					Send(&buff[i], &len);
					USB_WAIT_50ms();
				}

				/* Compare buff and hash to be equal */
				if (memcmp(buff, hash, sizeof(buff)) != 0) 
				{
					ret = ret_t::Error;
				}
			}
			else 
			{
				ret = ret_t::Error;
			}
		}
		else 
		{
			ret = ret_t::Error;
		}
		return ret;
	}

	ret_t ReadPinHash(uint8_t hash[16])
	{
		ret_t ret;
		uint8_t msg[8] = { CMD_READ_PIN_HASH, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t len = sizeof(msg);

		if (hash)
		{
			/* Send CMD */
			ret = Send(msg, &len);

			if (msg[0] == CMD_READ_PIN_HASH)
			{
				/* Read Pin Hash */
				for (uint8_t i = 0; i < 16; i += 8) 
				{
					len = 8;
					Receive(&hash[i], &len);
				}
			}
			else 
			{
				ret = ret_t::Error;
			}
		}
		else 
		{
			ret = ret_t::Error;
		}
		return ret;
	}

	ret_t WriteKeyboard(const uint8_t layout[128])
	{
		ret_t ret;
		uint8_t buff[128];

		uint8_t msg[8] = { CMD_WRITE_KEYBOARD, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t len = sizeof(msg);

		if (layout != NULL) 
		{
			/* Send CMD */
			ret = Send(msg, &len);
			memcpy(buff, layout, sizeof(buff));

			if (msg[0] == CMD_WRITE_KEYBOARD)
			{
				/* Send Keyboard Layout */
				for (uint8_t i = 0; i < sizeof(buff); i += 8) 
				{
					len = 8;
					Send(&buff[i], &len);
					USB_WAIT_50ms();
				}

				/* Compare buff and layout to be equal */
				if (memcmp(buff, layout, sizeof(buff)) != 0) 
				{
					ret = ret_t::Error;
				}
			}
			else 
			{
				ret = ret_t::Error;
			}
		}
		else 
		{
			ret = ret_t::Error;
		}
		return ret;
	}

	

	void PinToHash(uint16_t pin, uint8_t hash[16])
	{
		uint32_t key1[4];
		uint32_t key2[4];

		char pin_str[5] = "0000";

		/* Pin Range check */
		if ((pin < 0) || (pin > 9999)) 
		{
			/* BAD pin */
		}
		else 
		{
			/* Pin to string */
			sprintf(pin_str, "%04d", pin);
		}

		PinToKey((uint8_t*)pin_str, key1);
		PinToKey((uint8_t*)pin_str, key2);

		noekeon::Encrypt(key1, key2);
		memcpy(hash, key2, sizeof(key2));
	}

	uint16_t CredBuffSize(credential_t* list, uint16_t len)
	{
		size_t bytes = 0;
		for (uint16_t i = 0; i < len; i++) {
			bytes += strlen(list->name) + 1; // size of name + null
			bytes += sizeof(uint16_t);       // size of offset
			bytes += EncryptedSize(list);
			list++;
		}
		return (uint16_t)bytes;
	}

	uint16_t CredLen(uint8_t* buff, uint16_t size)
	{
		uint8_t* ptr = buff;
		uint16_t off = 0;
		uint16_t len = 0;
		
		if (ptr) 
		{
			do {
				ptr += strlen((char*)ptr) + 1;
				off = (ptr - buff) + 2;

				/* Check we do not exceed buffer size */
				if (off < size) 
				{
					/* Compute offset to find next credential */
					off = (ptr[0] | ptr[1] << 8);
					ptr = &buff[off];
					len++;
				}
			} while (off < size && off != 0);
		}
		return len;
	}

	ret_t Encrypt(credential_t* list, uint16_t len, uint8_t* buff, uint16_t size, uint16_t pin)
	{
		ret_t ret = ret_t::NoError;

		uint16_t enbytes;
		uint8_t* ptr = buff;
		uint8_t* enstart_ptr;
		uint16_t offset = 0;
		uint32_t key[4];
		char pin_str[5];

		/* Pin Range check */
		if ((pin < 0) || (pin > 9999)) 
		{
			ret = ret_t::Error;
		}
		else 
		{
			/* Pin to string */
			sprintf(pin_str, "%04d", pin);
			printf("%s -- PIN:%s\n", __FUNCTION__, pin_str);

			/* Compute key */
			PinToKey((uint8_t*)pin_str, key);

			/* Set Buffer to 0 */
			memset(buff, 0, size);

			/* Loop between credentials */
			for (uint16_t i = 0; i < len; i++)
			{
				strcpy((char*)ptr, list->name);
				ptr += strlen(list->name) + 1;

				if (i < (len - 1)) {
					/* Compute offset and add it to buffer */
					offset += CredBuffSize(list, 1);
				}
				else 
				{
					/* Last offset should be equal to 0 */
					offset = 0;
				}
				*ptr++ = uint8_t(offset);
				*ptr++ = uint8_t(offset >> 8);

				/* encrypt starts here */
				enstart_ptr = ptr;
				strcpy((char*)ptr, list->user);
				ptr += strlen(list->user) + 1;
				strcpy((char*)ptr, list->hop);
				ptr += strlen(list->hop) + 1;
				strcpy((char*)ptr, list->pass);
				ptr += strlen(list->pass) + 1;
				strcpy((char*)ptr, list->submit);
				ptr += strlen(list->submit) + 1;

				/* Number of Bytes to encrypt */
				enbytes = EncryptedSize(list);

				/* Point to next credential location */
				ptr = enstart_ptr + enbytes;

				/* Encrypt block by block */
				for (uint16_t j = 0; j < enbytes; j += 16, enstart_ptr += 16) 
				{
					noekeon::Encrypt(key, (uint32_t*)enstart_ptr);
				}

				list++;
			}
		}

		return ret;
	}

	ret_t Decrypt(credential_t* list, uint16_t len, uint8_t* buff, uint16_t size, uint16_t pin)
	{
		ret_t ret = ret_t::NoError;
		uint16_t enbytes;
		uint16_t offset;
		uint8_t* ptr = buff;
		uint8_t* destart_ptr;
		uint32_t key[4];
		char pin_str[5];

		/* Pin Range check */
		if ((pin < 0) || (pin > 9999)) 
		{
			ret = ret_t::Error;
		}
		else 
		{
			/* Pin to string */
			sprintf(pin_str, "%04d", pin);
			printf("%s -- PIN:%s\n", __FUNCTION__, pin_str);

			/* Compute key */
			PinToKey((uint8_t*)pin_str, key);

			for (uint16_t i = 0; i < len; i++) 
			{
				/* Save first credential */
				list->name = (char*)ptr;
				ptr += strlen(list->name) + 1;

				/* Check we do not exceed buffer size */
				if ((ptr - buff + 2) < size) 
				{
					/* Compute offset to find next credential */
					offset = ptr[0] | ptr[1] << 8;
					ptr += 2;

					/* Decrypt Start Ptr */
					destart_ptr = ptr;

					/*TODO: size check */
					if (offset == 0) 
					{
						enbytes = size - (ptr - buff);
					}
					else if (offset > (ptr - buff)) 
					{
						enbytes = offset - (ptr - buff);
					}
					else 
					{
						enbytes = 0;
					}

					/* Decrypt block by block */
					for (uint16_t j = 0; j < enbytes; j += 16, destart_ptr += 16) 
					{
						noekeon::Decrypt(key, (uint32_t*)destart_ptr);
					}

					/* Save decrypted credential information */
					list->user = (char*)ptr;
					ptr += strlen(list->user) + 1;
					list->hop = (char*)ptr;
					ptr += strlen(list->hop) + 1;
					list->pass = (char*)ptr;
					ptr += strlen(list->pass) + 1;
					list->submit = (char*)ptr;

					ptr = &buff[offset];
					list++;

				}
				else 
				{
					/* Escape from for loop */
					break;
				}
			}
		}
		return ret;
	}
}
