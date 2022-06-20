#pragma once

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdint.h>
#include "usbdrv/usbdrv.h"

#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

// hardware level
#include "USBProtocol.h"
#include "HIDDefines.h"
#include "HIDKeyboard.h"
#include "LedControl.h"
#include "MCUFlash.h"

// software level
#include "UserInput.h"
#include "UserOutput.h"
#include "PinCode.h"
#include "Credentials.h"

namespace main
{
    PROGMEM const char msgFirmware[] = "KeyKeeper 1.0";

    enum {
        STATE_TURNED_OFF,
        STATE_DISPLAY_FIRMWARE,
        STATE_ENTER_PIN_CODE,
        STATE_PIN_CODE_INVALID,
        STATE_CHOOSE_CREDENTIAL
    } state;

    char enteredPinCode[5];
    uint8_t pinCodeDigitIndex;

    ////////////////////////////////////////////////////////////////////////////////
    // Pin code input
    ////////////////////////////////////////////////////////////////////////////////

    bool IsPinCodeValid()
    {
        return strcmp_P(enteredPinCode, storedPinCode) == 0;
    }

    void PrintPinCode()
    {
        output::PrintMessage(F("PIN: "));
        output::PrintText(enteredPinCode);
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
        input::AllowSubmission(true);
        enteredPinCode[++pinCodeDigitIndex] = '0';
        enteredPinCode[pinCodeDigitIndex + 1] = 0;
        PrintPinCode();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Credential choose and submit
    ////////////////////////////////////////////////////////////////////////////////

    void PrintCredential()
    {
        output::PrintMessage(F("PRINT CREDENTIAL"));
    }

    void SubmitCredential()
    {
        output::PrintMessage(F("SUBMIT CREDENTIAL"));
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Go to next state
    ////////////////////////////////////////////////////////////////////////////////

    void GoToStateDisplayFirmware()
    {
        state = STATE_DISPLAY_FIRMWARE;
        input::AllowSubmission(true);
        output::PrintMessage(FPSTR(msgFirmware));
        led::SwitchBlinking();
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
        output::PrintMessage(F("PIN: INVALID"));
    }

    void GoToStateChooseCredential()
    {
        state = STATE_CHOOSE_CREDENTIAL;
        input::AllowSubmission(true);
        PrintCredential();
        led::SwitchOn();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Update current state
    ////////////////////////////////////////////////////////////////////////////////

    void UpdateStateTurnedOff(input::Action action)
    {
        if (action == input::ACTION_NEXT)
        {
            GoToStateDisplayFirmware();
        }
    }

    void UpdateStateDisplayFirmware(input::Action action)
    {
        if (action == input::ACTION_NEXT || action == input::ACTION_SUBMIT)
        {
            GoToStateEnterPinCode();
        }
    }

    void UpdateStateEnterPinCode(input::Action action)
    {
        if (action == input::ACTION_NEXT)
        {
            SwitchPinCodeDigit();
        }

        if (action == input::ACTION_SUBMIT)
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

    void UpdateStatePinCodeInvalid(input::Action action)
    {
        if (action == input::ACTION_NEXT)
        {
            GoToStateEnterPinCode();
        }
    }

    void UpdateStateChooseCredential(input::Action action)
    {
        if (action == input::ACTION_NEXT)
        {
            PrintCredential();
        }

        if (action == input::ACTION_SUBMIT)
        {
            SubmitCredential();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    void Init()
    {
        hid::Init();
        led::Init();
        input::Init();
        output::Init();
        state = STATE_TURNED_OFF;
    }

    void Update()
    {
        hid::Update();
        led::Update();
        input::Update();
        output::Update();
    }

    void ProcessAction(input::Action action)
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
}