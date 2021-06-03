
#include "src/keyboard.h"
#include "src/controls.h"
#include "src/display.h"

PROGMEM const char msgTurnOn[]  = "Turn On";
PROGMEM const char msgTurnOff[] = "Turn Off";
PROGMEM const char msgNext[]    = "Next";
PROGMEM const char msgSubmit[]  = "Submit";  

void ProcessAction(Controls::Action action)
{
    switch(action)
    {
        case Controls::ACTION_TURN_ON:
            digitalWrite(1, HIGH);
            Display::PrintMessage(FPSTR(msgTurnOn));
            break;

        case Controls::ACTION_TURN_OFF:
            digitalWrite(1, LOW);
            Display::PrintMessage(FPSTR(msgTurnOff));
            break;

        case Controls::ACTION_NEXT:
            //Controls::isPendingSubmit = false;
            Display::PrintMessage(FPSTR(msgNext));
            break;

        case Controls::ACTION_SUBMIT:
            Display::PrintMessage(FPSTR(msgSubmit));
            break;
    }
}

void setup()
{
    // Init onboard led
    pinMode(1, OUTPUT);
    digitalWrite(1, LOW);

    Keyboard::Init();
    Controls::Init();
}

void loop()
{
    Keyboard::Update();
    Controls::Update();
}
