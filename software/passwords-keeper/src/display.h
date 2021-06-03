#pragma once

#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

namespace Display
{
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
        CharToKey(ch, keycode, modifier);
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