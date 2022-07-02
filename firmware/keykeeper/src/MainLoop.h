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
#include "LedControl.h"
#include "USBProtocol.h"
#include "HIDDefines.h"
#include "HIDKeyboard.h"
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
        STATE_TURNING_ON,
        STATE_ENTER_PIN_CODE,
        STATE_CHECK_PIN_CODE,
        STATE_CHOOSE_CREDENTIAL,
        STATE_BLOCKED
    } state;

    char enteredPinCode[5];
    uint8_t pinCodeDigitIndex;
    uint8_t pinCodeEnterTries = 3;

    ////////////////////////////////////////////////////////////////////////////////
    // Pin code input
    ////////////////////////////////////////////////////////////////////////////////

    bool IsPinCodeValid()
    {
        return strcmp_P(enteredPinCode, storedPinCode) == 0;
    }

    void PrintPinCode()
    {
        output::PrintMessage(F("Pin: "));
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

    void GoToStateTurningOn()
    {
        state = STATE_TURNING_ON;
        input::AllowSubmission(true);
        output::PrintMessage(FPSTR(msgFirmware));
    }

    void GoToStateEnterPinCode()
    {
        state = STATE_ENTER_PIN_CODE;
        pinCodeDigitIndex = -1;
        GoToNextPinCodeDigitIndex();
    }

    void GoToStateCheckPinCode()
    {
        if (IsPinCodeValid())
        {
            state = STATE_CHECK_PIN_CODE;
            output::PrintMessage(F("Access: ALLOWED"));
            led::SwitchOn();
        }
        else if (--pinCodeEnterTries)
        {
            state = STATE_CHECK_PIN_CODE;
            output::PrintMessage(F("Access: DENIED"));
        }
        else
        {
            state = STATE_BLOCKED;
            output::PrintMessage(F("Access: BLOCKED"));
            led::SwitchBlinking();
        }
    }

    void GoToStateChooseCredential()
    {
        state = STATE_CHOOSE_CREDENTIAL;
        input::AllowSubmission(true);
        PrintCredential();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Update current state
    ////////////////////////////////////////////////////////////////////////////////

    void UpdateStateTurnedOff(input::Action action)
    {
        if (action == input::ACTION_NEXT)
        {
            GoToStateTurningOn();
        }
    }

    void UpdateStateTurningOn(input::Action action)
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
                GoToStateCheckPinCode();
            else
                GoToNextPinCodeDigitIndex();
        }
    }

    void UpdateStateCheckPinCode(input::Action action)
    {
        if (action == input::ACTION_NEXT)
        {
            if (IsPinCodeValid())
                GoToStateChooseCredential();
            else
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
        usb::Init();
        led::Init();
        input::Init();
        state = STATE_TURNED_OFF;
    }

    void Update()
    {
        usb::Update();
        led::Update();
        input::Update();
    }

    void ProcessAction(input::Action action)
    {
        switch (state)
        {
            case STATE_TURNED_OFF: UpdateStateTurnedOff(action); break;
            case STATE_TURNING_ON: UpdateStateTurningOn(action); break;
            case STATE_ENTER_PIN_CODE: UpdateStateEnterPinCode(action); break;
            case STATE_CHECK_PIN_CODE: UpdateStateCheckPinCode(action); break;
            case STATE_CHOOSE_CREDENTIAL: UpdateStateChooseCredential(action); break;
        }
    }
}