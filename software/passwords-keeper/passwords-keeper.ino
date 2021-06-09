
#include "src/hid_keyboard.h"
#include "src/hid_input.h"
#include "src/hid_output.h"

PROGMEM const char msgFirmware[]  = "Passwords Keeper 1.0";
PROGMEM const char storedPinCode[] = "2423";

enum
{
    STATE_TURNED_OFF,
    STATE_DISPLAY_FIRMWARE,
    STATE_ENTER_PIN_CODE,
    STATE_PIN_CODE_INVALID,
    STATE_CHOOSE_CREDENTIAL
} state;

char enteredPinCode[5];
byte pinCodeDigitIndex;

////////////////////////////////////////////////////////////////////////////////

bool IsPinCodeValid()
{
    return strcmp_P(enteredPinCode, storedPinCode) == 0;
}

void PrintPinCode()
{
    Output::PrintMessage(F("PIN: "));
    Output::PrintText(enteredPinCode);
}

void SwitchPinCodeDigit()
{
    if (++enteredPinCode[pinCodeDigitIndex] > '9')
    {
        enteredPinCode[pinCodeDigitIndex] = '0';
    }
    PrintPinCode();
}

void GoToNextPinCodeDigitIndex()
{
    pinCodeDigitIndex++;
    enteredPinCode[pinCodeDigitIndex] = '0';
    enteredPinCode[pinCodeDigitIndex + 1] = 0;
    PrintPinCode();
    Input::SetSubmit(true);
}

void PrintCredential()
{
    Output::PrintMessage(F("PRINT CREDENTIAL"));
}

void SubmitCredential()
{
    Output::PrintMessage(F("SUBMIT CREDENTIAL"));
}

////////////////////////////////////////////////////////////////////////////////

void GoToStateTurnedOff()
{
    Output::LedOff();
    state = STATE_TURNED_OFF;
    Output::PrintClear();
}

void GoToStateDisplayFirmware()
{
    Output::LedBlinking();
    state = STATE_DISPLAY_FIRMWARE;
    Output::PrintMessage(FPSTR(msgFirmware));
    Input::SetSubmit(true);
}

void GoToStateChooseCredential()
{
    Output::LedOn();
    state = STATE_CHOOSE_CREDENTIAL;
    PrintCredential();
    Input::SetSubmit(true);
}

void GoToStateEnterPinCode()
{
    state = STATE_ENTER_PIN_CODE;
    pinCodeDigitIndex = -1;
    GoToNextPinCodeDigitIndex();
}

void GoToStatePinCodeInvalid()
{
    state = STATE_PIN_CODE_INVALID;
    Output::PrintMessage(F("PIN: INVALID"));
}

////////////////////////////////////////////////////////////////////////////////

void StateTurnedOff(Input::Action action)
{
    if (action == Input::ACTION_TURN_ON)
    {
        GoToStateDisplayFirmware();
    }
}

void StateDisplayFirmware(Input::Action action)
{
    if (action == Input::ACTION_NEXT || action == Input::ACTION_SUBMIT)
    {
        if (IsPinCodeValid())
            GoToStateChooseCredential();
        else
            GoToStateEnterPinCode();
    }
}

void StateEnterPinCode(Input::Action action)
{
    if (action == Input::ACTION_NEXT)
    {
        SwitchPinCodeDigit();
    }

    if (action == Input::ACTION_SUBMIT)
    {
        if (pinCodeDigitIndex == 3)
        {
            if (IsPinCodeValid())
                GoToStateChooseCredential();
            else
                GoToStatePinCodeInvalid();
        }
        else
            GoToNextPinCodeDigitIndex();
    }
}

void StatePinCodeInvalid(Input::Action action)
{
    if (action == Input::ACTION_NEXT)
    {
        GoToStateEnterPinCode();
    }
}

void StateChooseCredential(Input::Action action)
{
    if (action == Input::ACTION_NEXT)
    {
        PrintCredential();
    }

    if (action == Input::ACTION_SUBMIT)
    {
        SubmitCredential();
    }
}

////////////////////////////////////////////////////////////////////////////////

void ProcessAction(Input::Action action)
{
    if (action == Input::ACTION_TURN_OFF)
    {
        GoToStateTurnedOff();
    }
    else
    {
        switch (state)
        {
            case STATE_TURNED_OFF: StateTurnedOff(action); break;
            case STATE_DISPLAY_FIRMWARE:StateDisplayFirmware(action); break;
            case STATE_ENTER_PIN_CODE: StateEnterPinCode(action); break;
            case STATE_PIN_CODE_INVALID: StatePinCodeInvalid(action); break;
            case STATE_CHOOSE_CREDENTIAL: StateChooseCredential(action); break;
        }
    }
}

void setup()
{
    state = STATE_TURNED_OFF;
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
