#pragma once

namespace led
{
    uint32_t switchTime;
    bool isLedOn;

    void SwitchOn()
    {
        switchTime = 0;
        isLedOn = true;
    }

    void SwitchOff()
    {
        switchTime = 0;
        isLedOn = false;
    }

    void SwitchBlinking()
    {
        switchTime = millis();
    }

    void Init()
    {
        pinMode(1, OUTPUT);
        SwitchOff();
    }

    void Update()
    {
        // if (switchTime)
        // {
        //     uint32_t nowTime = millis();
        //     if (nowTime - switchTime >= 1000)
        //     {
        //         switchTime = nowTime;
        //         isLedOn = !isLedOn;
        //     }
        // }

        digitalWrite(1, isLedOn ? HIGH : LOW);
    }
}
