#pragma once

namespace usb
{
    struct KeyboardReport
    {
        uint8_t modifier;
        uint8_t reserved;
        uint8_t keycode[6];
    };

    struct CustomReport
    {
        uint8_t data[8];
    }

    uint8_t idleRate;

    KeyboardReport keyboardReport;
    uint8_t keyboardLedsState;
    uint8_t keyboardLedsChange;

    CustomReport customReport;
}

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

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (usbRequest_t *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
    {
        switch (rq->bRequest)
        {
        case USBRQ_HID_GET_REPORT:
            // send "no keys pressed" if asked here
            usbMsgPtr = (usbMsgPtr_t)&usb::keyboardReport;
            usb::keyboardReport.modifier = 0;
            usb::keyboardReport.keycode[0] = 0;
            return sizeof(usb::keyboardReport);

        case USBRQ_HID_SET_REPORT:
            // if wLength == 1, should be LED state
            if (rq->wLength.word == 1)
            {
                return USB_NO_MSG;
            }
            break;

        case USBRQ_HID_GET_IDLE:
            // send idle rate to PC as required by spec
            usbMsgPtr = (usbMsgPtr_t)&usb::idleRate;
            return 1;

        case USBRQ_HID_SET_IDLE:
            // save idle rate from PC as required by spec
            usb::idleRate = rq->wValue.bytes[1];
            break;
        }
    }

    // by default don't return any data
    return 0;
}

usbMsgLen_t usbFunctionWrite(uint8_t *data, uint8_t len)
{
    if (usb::keyboardLedsState != data[0])
    {
        if (usb::keyboardLedsState != 0xFF)
        {
            usb::keyboardLedsChange = (usb::keyboardLedsState ^ data[0]);
        }
        usb::keyboardLedsState = data[0];
    }
    return 1; // Data read, not expecting more
}

namespace usb
{
    void Init()
    {
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

    void WaitForReady()
    {
        while (!usbInterruptIsReady())
        {
            Update();
            _delay_ms(5);
        }
    }

    void SendKeyboardReport()
    {
        usbSetInterrupt((uint8_t*)&keyboardReport, sizeof(keyboardReport));
    }

    void SendCustomReport()
    {
        usbSetInterrupt((uint8_t*)&customReport, sizeof(customReport));
    }
}
