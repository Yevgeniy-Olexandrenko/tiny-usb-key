#pragma once

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdint.h>
#include "usbdrv/usbdrv.h"

#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

#include "HIDDefines.h"
#include "HIDKeyboard.h"
#include "USBProtocol.h"

namespace main
{
    void Init()
    {

    }

    void Update()
    {

    }
}