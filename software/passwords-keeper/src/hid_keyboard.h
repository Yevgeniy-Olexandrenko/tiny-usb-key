#pragma once

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "hid_defines.h"

typedef unsigned char Byte;

/* PC's keyboard LED bits */
#define LED_NUM_LOCK       0x01
#define LED_CAPS_LOCK      0x02
#define LED_SCROLL_LOCK    0x04

/* Forward declarations */
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

    static Report report;     // sent to PC
    static LedState ledState; // received from PC
    static Byte idleRate;     // repeat rate for keyboards
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
    if (Byte ledChanges = (Keyboard::ledState ^ data[0]))
    {
        bool isPowerOn = (Keyboard::ledState == 0xFF);
        Keyboard::ledState = data[0];

        if (!isPowerOn)
        {
            if (ledChanges & LED_NUM_LOCK)
                NumLockToggle(Keyboard::ledState & LED_NUM_LOCK);

            if (ledChanges & LED_CAPS_LOCK)
                CapsLockToggle(Keyboard::ledState & LED_CAPS_LOCK);

            if (ledChanges & LED_SCROLL_LOCK)
                ScrollLockToggle(Keyboard::ledState & LED_SCROLL_LOCK);
        }
    }
    return 1; // Data read, not expecting more
}

namespace Keyboard
{
    void Init()
    {
        memset(&report, 0, sizeof(report));
        ledState = 0xFF;

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

    // Sends a key press only, with modifiers - no release
    // To release the key, send again with keyPress = 0
    void SendKeyPress(Byte keyPress, Byte modifiers)
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

    // Sends a key press AND release with modifiers
    void SendKeyStroke(byte keyStroke, byte modifiers)
    {
        SendKeyPress(keyStroke, modifiers);
        SendKeyPress(0, 0);
    }

    void SendKeyStroke(byte keyStroke)
    {
        SendKeyStroke(keyStroke, 0);
    }

    // Convert character to modifier + keycode
    void CharToKey(Byte ch, Byte& keycode, Byte& modifier)
    {
        if (ch >= '0' && ch <= '9')
        {
            modifier = 0;
            keycode  = (ch == '0') ? 39 : 30 + (ch - '1');
        }
        else if (ch >= 'a' && ch <= 'z')
        {
            modifier = (Keyboard::ledState & LED_CAPS_LOCK) ? MOD_SHIFT_LEFT : 0;
            keycode  = 4 + (ch - 'a');
        }
        else if (ch >= 'A' && ch <= 'Z')
        {
            modifier = (Keyboard::ledState & LED_CAPS_LOCK) ? 0 : MOD_SHIFT_LEFT;
            keycode  = 4 + (ch - 'A');
        }
        else
        {
            modifier = 0;
            keycode  = 0;
            switch (ch)
            {
            case '.':
                keycode = 0x37;
                break;
            case '_':
                modifier = MOD_SHIFT_LEFT;
            case '-':
                keycode = 0x2D;
                break;
            case ' ':
                keycode = 0x2C;
                break;
            case '\t':
                keycode = 0x2B;
                break;
            case '\n':
                keycode = 0x28;
                break;
            }
        }
    }
}