#pragma once

namespace Controls
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
void ProcessAction(Controls::Action action);

/* Handlers for PC's control keys */
void NumLockToggle(bool isOn)
{
}

void CapsLockToggle(bool isOn)
{
    if (Controls::isTurnedOn)
    {
        Controls::isPendingSubmit = true;
        Controls::pendingSubmitTime = millis();

        ProcessAction(Controls::ACTION_NEXT);
    }    
}

void ScrollLockToggle(bool isOn)
{
    if (isOn != Controls::isTurnedOn)
    {
        Controls::isPendingSubmit = false;

        if (Controls::isTurnedOn = isOn)
            ProcessAction(Controls::ACTION_TURN_ON);
        else
            ProcessAction(Controls::ACTION_TURN_OFF);
    }
}

namespace Controls
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
                    ProcessAction(Controls::ACTION_SUBMIT);
                }
            }            
        }
    }
}
