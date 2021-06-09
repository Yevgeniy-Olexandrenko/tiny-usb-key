#pragma once

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../usbdrv/usbdrv.h"
#include "hid_defines.h"

typedef unsigned char Byte;
typedef unsigned long Time;

/* PC's keyboard LED bits */
#define LED_NUM_LOCK       0x01
#define LED_CAPS_LOCK      0x02
#define LED_SCROLL_LOCK    0x04

/* Forward declarations */
void PowerOn();
void NumLockToggle(bool isOn);
void CapsLockToggle(bool isOn);
void ScrollLockToggle(bool isOn);

/* From Frank Zhao's USB Business Card project */
/* http://www.frank-zhao.com/cache/usbbusinesscard_details.php */
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = 
{
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x95, 0x08, //   REPORT_COUNT (8)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x01, //   LOGICAL_MAXIMUM (1)
    0x81, 0x02, //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x03, //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x05, //   REPORT_COUNT (5)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x05, 0x08, //   USAGE_PAGE (LEDs)
    0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05, //   USAGE_MAXIMUM (Kana)
    0x91, 0x02, //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x03, //   REPORT_SIZE (3)
    0x91, 0x03, //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06, //   REPORT_COUNT (6)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x65, //   LOGICAL_MAXIMUM (101)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))(0)
    0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00, //   INPUT (Data,Ary,Abs)
    0xc0        // END_COLLECTION
};

namespace Keyboard
{
    typedef struct
    {
        uint8_t modifier;
        uint8_t reserved;
        uint8_t keycode[6];
    } Report;

    typedef volatile Byte LedState;

    Report report;
    LedState ledState;
    LedState ledChanges;
    Byte idleRate;
}

usbMsgLen_t usbFunctionSetup(Byte data[8])
{
    usbRequest_t *rq = (usbRequest_t *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
    {
        switch (rq->bRequest)
        {
        case USBRQ_HID_GET_REPORT: // send "no keys pressed" if asked here
            // wValue: ReportType (highbyte), ReportID (lowbyte)
            usbMsgPtr = (usbMsgPtr_t)&Keyboard::report; // we only have this one
            Keyboard::report.modifier = 0;
            Keyboard::report.keycode[0] = 0;
            return sizeof(Keyboard::report);

        case USBRQ_HID_SET_REPORT: // if wLength == 1, should be LED state
            return (rq->wLength.word == 1) ? USB_NO_MSG : 0;

        case USBRQ_HID_GET_IDLE: // send idle rate to PC as required by spec
            usbMsgPtr = (usbMsgPtr_t)&Keyboard::idleRate;
            return 1;

        case USBRQ_HID_SET_IDLE: // save idle rate as required by spec
            Keyboard::idleRate = rq->wValue.bytes[1];
            return 0;
        }
    }
    return 0; // by default don't return any data
}

usbMsgLen_t usbFunctionWrite(Byte *data, Byte len)
{
    if (Keyboard::ledChanges = (Keyboard::ledState ^ data[0]))
    {
        bool isPowerOn = (Keyboard::ledState == 0xFF);
        Keyboard::ledState = data[0];

        if (isPowerOn) PowerOn();
    }
    return 1; // Data read, not expecting more
}

namespace Keyboard
{
    void Init()
    {
        memset(&report, 0, sizeof(report));
        ledState = 0xFF;
        ledChanges = 0x00;

        wdt_enable(WDTO_1S);
        usbInit();

        usbDeviceDisconnect();
        for (Byte i = 0; i < 250; i++)
        {
            wdt_reset();
            _delay_ms(2);
        }
        usbDeviceConnect();
        sei();
    }

    void Update()
    {
        wdt_reset();
        usbPoll();
    }

    // delay while updating until we are finished delaying
    void Delay(long ms) 
    {
        Time last = millis();
        while (ms > 0) 
        {
            Time now = millis();
            ms -= now - last;
            last = now;
            Update();
        }
    }

    // Sends a key press only, with modifiers - no release
    // To release the key, send again with keyPress = 0
    void KeyPress(Byte keyPress, Byte modifiers)
    {
        while (!usbInterruptIsReady())
        {
            // We wait until we can send keyPress, so 
            // we know the previous keyPress was sent.
            Update();
            _delay_ms(5);
        }

        report.keycode[0] = keyPress;
        report.modifier = modifiers;

        usbSetInterrupt((Byte*)&report, sizeof(report));
    }

    void KeyRelease()
    {
        KeyPress(0, 0);
    }

    // Sends a key press AND release with modifiers
    void KeyStroke(byte keyStroke, byte modifiers)
    {
        KeyPress(keyStroke, modifiers);
        KeyRelease();
    }

    void KeyStroke(byte keyStroke)
    {
        KeyStroke(keyStroke, 0);
    }

    // Read PC Keyboard LEDs status
    bool IsPCLedChanged(Byte ledMask) { return (ledChanges & ledMask); }
    bool IsPCLedOn(Byte ledMask) { return (ledState & ledMask); }
    void ConsumePCLedChanges() { ledChanges = 0; }

    // Convert character to modifier + keycode
    void CharToKey(Byte ch, Byte& keycode, Byte& modifier)
    {
        if (ch >= 0x00 && ch <= 0x7F)
        {
            ch = pgm_read_byte(asciimap + ch);

            modifier = ch & SHIFT ? KEY_MOD_LSHIFT : 0;
            keycode = ch & ~SHIFT;

            if (keycode >= KEY_A && keycode <= KEY_Z && IsPCLedOn(LED_CAPS_LOCK))
            {
                modifier ^= KEY_MOD_LSHIFT;
            }
        }
        else
        {
            modifier = 0;
            keycode = KEY_NONE;
        }
    }
}