#pragma once

namespace hid
{
    enum {
        LED_NUM_LOCK    = 0x01,
        LED_CAPS_LOCK   = 0x02,
        LED_SCROLL_LOCK = 0x04
    };

    void Init()
    {
        memset(&usb::keyboardReport, 0, sizeof(usb::keyboardReport));
        usb::keyboardLedsState  = 0xFF;
        usb::keyboardLedsChange = 0x00;
        usb::Init();
    }

    void Update()
    {
        usb::Update();
    }

    // Sends a key press only with modifiers, no release
    void KeyPress(uint8_t key, uint8_t mod)
    {
        // We wait until we can send key press, so 
        // we know the previous key press was sent

        usb::WaitForReady();
        usb::keyboardReport.modifier = mod;
        usb::keyboardReport.keycode[0] = key;
        usb::SendKeyboardReport();
    }

    void KeyRelease()
    {
        KeyPress(0, 0);
    }

    // Sends a key press AND release with modifiers
    void KeyStroke(uint8_t key, uint8_t mod)
    {
        KeyPress(key, mod);
        KeyRelease();
    }

    void KeyStroke(uint8_t key)
    {
        KeyStroke(key, 0);
    }

    // Read PC Keyboard LEDs status
    bool IsPCLedChanged(uint8_t mask) { return (usb::keyboardLedsChange & mask); }
    bool IsPCLedOn(uint8_t mask) { return (usb::keyboardLedsState & mask); }
    void ConsumePCLedChanges() { usb::keyboardLedsChange = 0x00; }

    // Convert character to modifier + keycode
    void CharToKey(uint8_t ch, uint8_t& key, uint8_t& mod)
    {
        if (ch >= 0x00 && ch <= 0x7F)
        {
            ch = pgm_read_byte(asciimap + ch);

            mod = ch & SHIFT ? KEY_MOD_LSHIFT : 0;
            key = ch & ~SHIFT;

            if (key >= KEY_A && key <= KEY_Z && IsPCLedOn(LED_CAPS_LOCK))
            {
                mod ^= KEY_MOD_LSHIFT;
            }
        }
        else
        {
            mod = 0;
            key = KEY_NONE;
        }
    }
}
