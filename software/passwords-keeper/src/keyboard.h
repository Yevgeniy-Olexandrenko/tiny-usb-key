#pragma once

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usbdrv/usbdrv.h"

typedef unsigned char Byte;

/* PC's keyboard LED bits */
#define LED_NUM_LOCK       0x01
#define LED_CAPS_LOCK      0x02
#define LED_SCROLL_LOCK    0x04

/* Key modificatos declaration */
#define MOD_CONTROL_LEFT   (1<<0)
#define MOD_SHIFT_LEFT     (1<<1)
#define MOD_ALT_LEFT       (1<<2)
#define MOD_GUI_LEFT       (1<<3)
#define MOD_CONTROL_RIGHT  (1<<4)
#define MOD_SHIFT_RIGHT    (1<<5)
#define MOD_ALT_RIGHT      (1<<6)
#define MOD_GUI_RIGHT      (1<<7)


/* Keyboard usage values, see usb.org's HID-usage-tables document, */
/* chapter 10 Keyboard/Keypad Page for more codes. */
#define KEY_A          4
#define KEY_B          5
#define KEY_C          6
#define KEY_D          7
#define KEY_E          8
#define KEY_F          9
#define KEY_G          10
#define KEY_H          11
#define KEY_I          12
#define KEY_J          13
#define KEY_K          14
#define KEY_L          15
#define KEY_M          16
#define KEY_N          17
#define KEY_O          18
#define KEY_P          19
#define KEY_Q          20
#define KEY_R          21
#define KEY_S          22
#define KEY_T          23
#define KEY_U          24
#define KEY_V          25
#define KEY_W          26
#define KEY_X          27
#define KEY_Y          28
#define KEY_Z          29
#define KEY_1          30
#define KEY_2          31
#define KEY_3          32
#define KEY_4          33
#define KEY_5          34
#define KEY_6          35
#define KEY_7          36
#define KEY_8          37
#define KEY_9          38
#define KEY_0          39
#define KEY_ENTER      40
#define KEY_BACKSPACE  42
#define KEY_SPACE      44
#define KEY_F1         58
#define KEY_F2         59
#define KEY_F3         60
#define KEY_F4         61
#define KEY_F5         62
#define KEY_F6         63
#define KEY_F7         64
#define KEY_F8         65
#define KEY_F9         66
#define KEY_F10        67
#define KEY_F11        68
#define KEY_F12        69
#define KEY_HOME       74
#define KEY_DELETE     76
#define KEY_END        77
#define KEY_ARROW_R    79
#define KEY_ARROW_L    80
#define KEY_ARROW_D    81
#define KEY_ARROW_U    82

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
}