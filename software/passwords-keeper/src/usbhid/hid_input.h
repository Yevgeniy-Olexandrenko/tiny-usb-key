#pragma once

void PowerOn()
{
    Keyboard::KeyRelease();
    if (Keyboard::IsPCLedOn(LED_SCROLL_LOCK)) Keyboard::KeyStroke(KEY_SCROLLLOCK);
    if (Keyboard::IsPCLedOn(LED_CAPS_LOCK)) Keyboard::KeyStroke(KEY_CAPSLOCK);
}

namespace Input
{
    enum Action
    {
        ACTION_NONE,
        ACTION_TURN_ON,
        ACTION_TURN_OFF,
        ACTION_NEXT,
        ACTION_SUBMIT
    };

    bool isTurnedOn;
    Time pendingSubmitTime;
}

void ProcessAction(Input::Action action);

namespace Input
{
    void Init()
    {
        isTurnedOn = false;
    }

    void AllowSubmit(bool yes)
    {
        pendingSubmitTime = yes ? millis() : 0;
    }

    void Update()
    {
        if (Keyboard::IsPCLedChanged(LED_SCROLL_LOCK))
        {
            bool isScrollLockOn = Keyboard::IsPCLedOn(LED_SCROLL_LOCK);
            if (isTurnedOn != isScrollLockOn)
            {
                AllowSubmit(false);
                isTurnedOn = isScrollLockOn;
                ProcessAction(isTurnedOn ? ACTION_TURN_ON : ACTION_TURN_OFF);
            }
        }

        else if (isTurnedOn)
        {
            if (Keyboard::IsPCLedChanged(LED_CAPS_LOCK))
            {
                AllowSubmit(true);
                ProcessAction(ACTION_NEXT);
            }

            else if (pendingSubmitTime)
            {
                Time nowTime = millis();
                if (nowTime - pendingSubmitTime >= 2000)
                {
                    AllowSubmit(false);
                    ProcessAction(ACTION_SUBMIT);
                }
            }            
        }

        Keyboard::ConsumePCLedChanges();
    }
}
