#pragma once

#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

namespace Output
{
    void Clear()
    {
        Keyboard::SendKeyStroke(0);
#if 0
        Keyboard::SendKeyStroke(KEY_END);
        Keyboard::SendKeyStroke(KEY_HOME, MOD_SHIFT_LEFT);
        Keyboard::SendKeyStroke(KEY_DELETE);
#else
        Keyboard::SendKeyStroke(KEY_HOME);
        Keyboard::SendKeyStroke(KEY_END, MOD_SHIFT_LEFT);
        Keyboard::SendKeyStroke(KEY_BACKSPACE);
#endif
    }

    void PrintChar(char ch)
    {
        Byte keycode, modifier;
        Keyboard::CharToKey(ch, keycode, modifier);
        Keyboard::SendKeyStroke(keycode, modifier);
    }

    void PrintText(const __FlashStringHelper* str)
    {
        const char* ptr = (const char*)str;
        for (char ch; ch = pgm_read_byte(ptr); ++ptr) PrintChar(ch);
    }

    void PrintMessage(const __FlashStringHelper* str)
    {
        Clear();
        PrintText(str);
    }
}