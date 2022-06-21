#pragma once

namespace usb
{
    enum
    {
        REPORT_KEYBOARD = 1,
        REPORT_CUSTOM   = 2,
    };

    struct KeyboardReport
    {
        uint8_t reportId;
        uint8_t modifier;
        uint8_t keycode;
    };

    struct CustomReport
    {
        uint8_t data[8];
    };

    uint8_t reportId;
    uint8_t idleRate  = 500 / 4; // see HID1_11.pdf sect 7.2.4
    uint8_t protocolVersion = 0; // see HID1_11.pdf sect 7.2.6

    KeyboardReport keyboardReport;
    uint8_t keyboardLedsState;
    uint8_t keyboardLedsChange;

    CustomReport customReport;
}

/* From Frank Zhao's USB Business Card project */
/* http://www.frank-zhao.com/cache/usbbusinesscard_details.php */
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = 
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0x65,        //   Usage Maximum (0x65)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x03,        //   Report Count (3)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x03,        //   Usage Maximum (Scroll Lock)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x75, 0x05,        //   Report Size (5)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        // Usage (0x01)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x08,        //   Report Count (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x09, 0x00,        //   Usage (0x00)
    0xB2, 0x02, 0x01,  //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
    0xC0,              // End Collection
};

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (usbRequest_t *)data;
    usb::reportId = rq->wValue.bytes[0];

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
    {
        switch (rq->bRequest)
        {
        case USBRQ_HID_GET_REPORT:
            if (usb::reportId == usb::REPORT_KEYBOARD)
            {
                usbMsgPtr = (usbMsgPtr_t)&usb::keyboardReport;
                return sizeof(usb::keyboardReport);
            }
            break;

        case USBRQ_HID_SET_REPORT:
            if (usb::reportId == usb::REPORT_KEYBOARD)
            {
                return USB_NO_MSG;
            }
            break;

        case USBRQ_HID_GET_IDLE:
            usbMsgPtr = (usbMsgPtr_t)&usb::idleRate;
            return 1;

        case USBRQ_HID_SET_IDLE:
            usb::idleRate = rq->wValue.bytes[1];
            break;

        case USBRQ_HID_GET_PROTOCOL:
            usbMsgPtr = (usbMsgPtr_t)&usb::protocolVersion;
            return 1;

        case USBRQ_HID_SET_PROTOCOL:
            usb::protocolVersion = rq->wValue.bytes[1];
            break;
        }
    }
    return 0;
}

usbMsgLen_t usbFunctionWrite(uint8_t *data, uint8_t len)
{
    if (usb::reportId == usb::REPORT_KEYBOARD)
    {
        // if (data[1])
        //     led::SwitchOn();
        // else
        //     led::SwitchOff();

        if (usb::keyboardLedsState != data[1])
        {
            if (usb::keyboardLedsState != 0xFF)
            {
                usb::keyboardLedsChange = (usb::keyboardLedsState ^ data[1]);
            }
            usb::keyboardLedsState = data[1];
        }
    }
    else if (usb::reportId == usb::REPORT_CUSTOM)
    {
        // TODO
    }
    return 1;
}

usbMsgLen_t usbFunctionRead(uint8_t *data, uint8_t len)
{
    if (usb::reportId == usb::REPORT_CUSTOM)
    {
        // TODO
    }
    return 0;
}

namespace usb
{
    void Init()
    {
        keyboardReport.reportId = REPORT_KEYBOARD;
        keyboardLedsState  = 0xFF;
        keyboardLedsChange = 0x00;

        wdt_enable(WDTO_1S);
        usbInit();
        usbDeviceDisconnect();

        wdt_reset();
        _delay_ms(250);
        usbDeviceConnect();
        sei();
    }

    void Update()
    {
        wdt_reset();
        usbPoll();
    }

    void WaitForReadiness()
    {
        while (!usbInterruptIsReady()) Update();
    }

    void SendKeyboardReport()
    {
        usbSetInterrupt((uint8_t*)&keyboardReport, sizeof(keyboardReport));
    }
}
