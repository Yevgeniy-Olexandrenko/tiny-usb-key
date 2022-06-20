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

void ProcessAction(input::Action action);

namespace input
{
    void Init()
    {
        // do nothing
    }

    void AllowSubmission(bool yes)
    {
        pendingSubmitTime = (yes ? millis() : 0);
    }

    void Update()
    {
        if (hid::IsPCLedChanged(hid::LED_CAPS_LOCK))
        {
            AllowSubmission(true);
            ProcessAction(input::ACTION_NEXT);
        }

        else if (pendingSubmitTime)
        {
            uint32_t nowTime = millis();
            if (nowTime - pendingSubmitTime >= 2000)
            {
                AllowSubmission(false);
                ProcessAction(input::ACTION_SUBMIT);
            }
        }

        hid::ConsumePCLedChanges();
    }
}
