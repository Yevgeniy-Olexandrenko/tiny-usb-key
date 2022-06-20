#pragma once

namespace output
{
    void PrintClear()
    {
        hid::KeyRelease();
        hid::KeyStroke(KEY_HOME);
        hid::KeyStroke(KEY_END, KEY_MOD_LSHIFT);
        hid::KeyStroke(KEY_BACKSPACE);
    }

    void PrintChar(char ch)
    {
        uint8_t key, mod;
        hid::CharToKey(ch, key, mod);
        hid::KeyStroke(key, mod);
    }

    void PrintText(const char* str)
    {
        for (char ch; ch = *str; ++str) PrintChar(ch);
    }

    void PrintText(const __FlashStringHelper* str)
    {
        const char* ptr = (const char*)str;
        for (char ch; ch = pgm_read_byte(ptr); ++ptr) PrintChar(ch);
    }

    void PrintMessage(const __FlashStringHelper* str)
    {
        PrintClear();
        PrintText(str);
    }
}
