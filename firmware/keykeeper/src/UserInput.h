#pragma once

namespace input
{
    enum Action
    {
        ACTION_NONE,
        ACTION_NEXT,
        ACTION_SUBMIT
    };

    uint32_t pendingSubmitTime;
}

namespace main
{
    void ProcessAction(input::Action action);
}

namespace input
{
    
    void AllowSubmission(bool yes)
    {
        pendingSubmitTime = (yes ? millis() : 0);
    }

    void Init()
    {
        AllowSubmission(false);
    }

    void Update()
    {
        Action action = ACTION_NONE;

        if (hid::IsPCLedChanged(hid::LED_CAPS_LOCK))
        {
            AllowSubmission(true);
            action = input::ACTION_NEXT;
        }

        else if (pendingSubmitTime)
        {
            uint32_t nowTime = millis();
            if (nowTime - pendingSubmitTime >= 2000)
            {
                AllowSubmission(false);
                action = input::ACTION_SUBMIT;
            }
        }

        hid::ConsumePCLedChanges();
        main::ProcessAction(action);
    }
}
