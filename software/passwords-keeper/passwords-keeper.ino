
#include "src/hid_keyboard.h"
#include "src/hid_input.h"
#include "src/hid_output.h"

PROGMEM const char msgTurnOn[]  = "Turn On";
PROGMEM const char msgTurnOff[] = "Turn Off";
PROGMEM const char msgNext[]    = "Next";
PROGMEM const char msgSubmit[]  = "Submit";  

void ProcessAction(Input::Action action)
{
    switch(action)
    {
        case Input::ACTION_TURN_ON:
            Output::LedBlinking();
            Output::PrintMessage(FPSTR(msgTurnOn));
            break;

        case Input::ACTION_TURN_OFF:
            Output::LedOff();
            Output::PrintMessage(FPSTR(msgTurnOff));
            break;

        case Input::ACTION_NEXT:
            //Input::isPendingSubmit = false;
            Output::PrintMessage(FPSTR(msgNext));
            break;

        case Input::ACTION_SUBMIT:
            Output::PrintMessage(FPSTR(msgSubmit));
            break;
    }
}

void setup()
{
    Keyboard::Init();
    Input::Init();
    Output::Init();
}

void loop()
{
    Keyboard::Update();
    Input::Update();
    Output::Update();
}
