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

    typedef unsigned long Time;

    bool isTurnedOn;
    bool isPendingSubmit;
    Time pendingSubmitTime;
}

/* Forward declarations */
void ProcessAction(Input::Action action);

/* Handlers for PC's control keys */
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
                if (nowTime - pendingSubmitTime >= 3000)
                {
                    isPendingSubmit = false;
                    ProcessAction(Input::ACTION_SUBMIT);
                }
            }            
        }
    }
}
