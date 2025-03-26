/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gpiointerrupt.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>
#include "ti_drivers_config.h"  // Board-specific configurations

// Morse code for "SOS" and "OK"
const char* SOS = "...---...";
const char* OK = "---.-";  // Morse code for "OK"
const char* currentMessage = "...---...";  // Start with "SOS"
int currentIndex = 0;  // Current position in the Morse code string

// Morse code state machine states
enum MorseStates { DOT, DASH, INTER_CHAR, INTER_WORD } state = DOT;
int buttonPressed = 0;  // Button press flag
int tickCounter = 0;  // Counter for timing control

/*
 * Timer callback: Called every 500ms to trigger the state machine (synchronous clock)
 */
void timerCallback(Timer_Handle myHandle, int_fast16_t status) {
    // Increment the tick counter
    tickCounter++;

    switch (state) {
        case DOT:
            if (tickCounter == 1) {  // DOT lasts for 1 tick (500ms)
                GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);  // Red LED for DOT
            } else if (tickCounter >= 2) {
                GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
                state = INTER_CHAR;
                tickCounter = 0;  // Reset tick counter for the next state
            }
            break;

        case DASH:
            if (tickCounter == 1) {  // DASH lasts for 3 ticks (1500ms)
                GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);  // Green LED for DASH
            } else if (tickCounter >= 4) {
                GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
                state = INTER_CHAR;
                tickCounter = 0;  // Reset tick counter for the next state
            }
            break;

        case INTER_CHAR:
            if (tickCounter >= 1) {  // INTER_CHAR lasts for 1 tick (500ms)
                currentIndex++;
                if (currentMessage[currentIndex] == '\0') {
                    state = INTER_WORD;  // End of message, go to word gap
                } else {
                    state = (currentMessage[currentIndex] == '.') ? DOT : DASH;  // DOT or DASH for next char
                }
                tickCounter = 0;  // Reset tick counter for the next state
            }
            break;

        case INTER_WORD:
            if (tickCounter >= 7) {  // INTER_WORD lasts for 7 ticks (3500ms)
                currentIndex = 0;  // Reset to start of message

                // Check if button was pressed
                if (buttonPressed) {
                    currentMessage = (currentMessage == SOS) ? OK : SOS;  // Toggle message
                    buttonPressed = 0;  // Reset button flag
                }

                // Start next message
                state = (currentMessage[currentIndex] == '.') ? DOT : DASH;
                tickCounter = 0;  // Reset tick counter for the next state
            }
            break;
    }
}

/*
 * Initialize and start the timer
 */
void initTimer(void) {
    Timer_Handle timer;
    Timer_Params params;

    // Initialize the timer
    Timer_init();
    Timer_Params_init(&params);
    params.period = 500000;  // 500ms period
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;  // Call the callback repeatedly
    params.timerCallback = timerCallback;  // Set the callback function

    // Open and start the timer
    timer = Timer_open(CONFIG_TIMER_0, &params);
    if (timer == NULL) {
        while (1) {}  // Timer failed to open
    }

    if (Timer_start(timer) == Timer_STATUS_ERROR) {
        while (1) {}  // Timer failed to start
    }
}

/*
 * Button press callback for CONFIG_GPIO_BUTTON_0
 */
void gpioButtonFxn0(uint_least8_t index) {
    buttonPressed = 1;  // Set flag to toggle the message
}

/*
 * Button press callback for CONFIG_GPIO_BUTTON_1
 */
void gpioButtonFxn1(uint_least8_t index) {
    buttonPressed = 1;  // Set flag to toggle the message
}

/*
 * Main entry point
 */
void *mainThread(void *arg0) {
    // Initialize GPIO and Timer
    GPIO_init();
    initTimer();

    // Configure the LED pins
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);  // Red LED
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);  // Green LED

    // Configure the button pins
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    // Install button callbacks
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);
    GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn1);

    // Enable button interrupts
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_1);

    // No need for a loop; the timer and interrupts handle everything
    return NULL;
}

