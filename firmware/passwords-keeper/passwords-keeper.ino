
#include "src/usbhid/hid_keyboard.h"
#include "src/usbhid/hid_input.h"
#include "src/usbhid/hid_output.h"

#include "src/credentials_builder.h"
#include "credentials.h"

PROGMEM const char msgFirmware[] = "Passwords Keeper 1.0";

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
// Pin code input
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
    Input::AllowSubmission(true);
    enteredPinCode[++pinCodeDigitIndex] = '0';
    enteredPinCode[pinCodeDigitIndex + 1] = 0;
    PrintPinCode();
}

////////////////////////////////////////////////////////////////////////////////
// Credential choose and submit
////////////////////////////////////////////////////////////////////////////////

void PrintCredential()
{
    Output::PrintMessage(F("PRINT CREDENTIAL"));
}

void SubmitCredential()
{
    Output::PrintMessage(F("SUBMIT CREDENTIAL"));
}

////////////////////////////////////////////////////////////////////////////////
// Go to next state
////////////////////////////////////////////////////////////////////////////////

void GoToStateDisplayFirmware()
{
    state = STATE_DISPLAY_FIRMWARE;
    Input::AllowSubmission(true);
    Output::LedLocked();
    Output::PrintMessage(FPSTR(msgFirmware));
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

void GoToStateChooseCredential()
{
    state = STATE_CHOOSE_CREDENTIAL;
    Input::AllowSubmission(true);
    Output::LedUnlocked();
    PrintCredential();
}

////////////////////////////////////////////////////////////////////////////////
// Update current state
////////////////////////////////////////////////////////////////////////////////

void UpdateStateTurnedOff(Input::Action action)
{
    if (action == Input::ACTION_NEXT)
    {
        GoToStateDisplayFirmware();
    }
}

void UpdateStateDisplayFirmware(Input::Action action)
{
    if (action == Input::ACTION_NEXT || action == Input::ACTION_SUBMIT)
    {
        GoToStateEnterPinCode();
    }
}

void UpdateStateEnterPinCode(Input::Action action)
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

void UpdateStatePinCodeInvalid(Input::Action action)
{
    if (action == Input::ACTION_NEXT)
    {
        GoToStateEnterPinCode();
    }
}

void UpdateStateChooseCredential(Input::Action action)
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
    switch (state)
    {
        case STATE_TURNED_OFF: UpdateStateTurnedOff(action); break;
        case STATE_DISPLAY_FIRMWARE:UpdateStateDisplayFirmware(action); break;
        case STATE_ENTER_PIN_CODE: UpdateStateEnterPinCode(action); break;
        case STATE_PIN_CODE_INVALID: UpdateStatePinCodeInvalid(action); break;
        case STATE_CHOOSE_CREDENTIAL: UpdateStateChooseCredential(action); break;
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
