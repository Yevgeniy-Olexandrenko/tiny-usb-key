#pragma once

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
    bool isPendingSubmit;
    Time pendingSubmitTime;
}

/* Forward declarations */
void ProcessAction(Input::Action action);

/* Handlers for PC's control keys */
void PowerOn()
{
    Keyboard::SendKeyStroke(KEY_NONE);
    if (Keyboard::IsKeyLedOn(LED_SCROLL_LOCK)) Keyboard::SendKeyStroke(KEY_SCROLLLOCK);
    if (Keyboard::IsKeyLedOn(LED_CAPS_LOCK)) Keyboard::SendKeyStroke(KEY_CAPSLOCK);
}

void NumLockToggle(bool isOn)
{
    // do nothing
}

void CapsLockToggle(bool isOn)
{
    if (Input::isTurnedOn)
    {
        Input::isPendingSubmit = true;
        Input::pendingSubmitTime = millis();

        ProcessAction(Input::ACTION_NEXT);
    }    
}

void ScrollLockToggle(bool isOn)
{
    if (isOn != Input::isTurnedOn)
    {
        Input::isPendingSubmit = false;

        if (Input::isTurnedOn = isOn)
            ProcessAction(Input::ACTION_TURN_ON);
        else
            ProcessAction(Input::ACTION_TURN_OFF);
    }
}

namespace Input
{
    void Init()
    {
        isTurnedOn = false;
    }

    void Update()
    {
        if (isTurnedOn)
        {
            if (isPendingSubmit)
            {
                Time nowTime = millis();
                if (nowTime - pendingSubmitTime >= 2000)
                {
                    isPendingSubmit = false;
                    ProcessAction(Input::ACTION_SUBMIT);
                }
            }            
        }
    }
}
