#pragma once

#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

namespace Output
{
    Time ledSwitchTime;
    bool isLedOn;

    void PrintClear()
    {
        Keyboard::KeyRelease();
        Keyboard::KeyStroke(KEY_HOME);
        Keyboard::KeyStroke(KEY_END, KEY_MOD_LSHIFT);
        Keyboard::KeyStroke(KEY_BACKSPACE);
    }

    void PrintChar(char ch)
    {
        Byte keycode, modifier;
        Keyboard::CharToKey(ch, keycode, modifier);
        Keyboard::KeyStroke(keycode, modifier);
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

    void LedOn()
    {
        ledSwitchTime = 0;
        isLedOn = true;
    }

    void LedOff()
    {
        ledSwitchTime = 0;
        isLedOn = false;
    }

    void LedBlinking()
    {
        ledSwitchTime = millis();
    }

    void LedLocked()
    {
        LedBlinking();
    }

    void LedUnlocked()
    {
        LedOn();
    }

     void Init()
    {
        pinMode(1, OUTPUT);
        LedOff();
    }

    void Update()
    {
        if (ledSwitchTime)
        {
            Time nowTime = millis();
            if (nowTime - ledSwitchTime >= 1000)
            {
                ledSwitchTime = nowTime;
                isLedOn = !isLedOn;
            }
        }

        digitalWrite(1, isLedOn ? HIGH : LOW);
    }
}