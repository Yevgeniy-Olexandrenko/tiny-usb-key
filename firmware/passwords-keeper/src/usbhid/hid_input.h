#pragma once

namespace Input
{
    enum Action
    {
        ACTION_NONE,
        ACTION_NEXT,
        ACTION_SUBMIT
    };

    Time pendingSubmitTime;
}

void ProcessAction(Input::Action action);

namespace Input
{
    void Init()
    {
    }

    void AllowSubmission(bool yes)
    {
        pendingSubmitTime = yes ? millis() : 0;
    }

    void Update()
    {
        if (Keyboard::IsPCLedChanged(LED_CAPS_LOCK))
        {
            AllowSubmission(true);
            ProcessAction(ACTION_NEXT);
        }

        else if (pendingSubmitTime)
        {
            Time nowTime = millis();
            if (nowTime - pendingSubmitTime >= 2000)
            {
                AllowSubmission(false);
                ProcessAction(ACTION_SUBMIT);
            }
        }

        Keyboard::ConsumePCLedChanges();
    }
}
