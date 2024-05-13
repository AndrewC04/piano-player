/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// define GPIO pins
#define BUZZER_PIN GPIO_PORT_P2, GPIO_PIN1
#define LED_PIN GPIO_PORT_P2, GPIO_PIN1

// define tone frequencies (4th octave)
#define T_C 262
#define T_D 294
#define T_E 330
#define T_F 349
#define T_G 392
#define T_A 440
#define T_B 493

// tone functions
void tone(uint32_t frequency);
void noTone(void);

void main(void) {
    /* Stop Watchdog */
    MAP_WDT_A_holdTimer();

    // initialize GPIO
    MAP_GPIO_setAsOutputPin(LED_PIN);
    MAP_GPIO_setOutputLowOnPin(LED_PIN);

    // initialize GPIO input pins
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0); // T_C pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN1); // T_D pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2); // T_E pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN3); // T_F pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN4); // T_G pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN5); // T_A pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN6); // T_B pin

    while (1)
    {
        // T_C pin
        if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0))
        {
            tone(T_C);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN1))
        {
            // T_D pin
            tone(T_D);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2))
        {
            // T_E pin
            tone(T_E);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3))
        {
            // T_F pin
            tone(T_F);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN4))
        {
            // T_G pin
            tone(T_G);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5))
        {
            // T_A pin
            tone(T_A);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6))
        {
            // T_B pin
            tone(T_B);
            MAP_GPIO_setOutputHighOnPin(LED_PIN);
        }
        else
        {
            // return
            noTone();
            MAP_GPIO_setOutputLowOnPin(LED_PIN);
        }
    }
}

// Function to generate a tone of a given frequency (in Hz)
void tone(uint32_t frequency) {
    // Calculate half-period (in microseconds)
    uint32_t half_period_us = 500000 / frequency;

    // Toggle buzzer pin
    while (1) {
        MAP_GPIO_setOutputHighOnPin(BUZZER_PIN);
        // Delay for half the period
        uint32_t delay_cycles = half_period_us * (SystemCoreClock / 1000000);
        uint32_t i = 0;
        for (i = 0; i < delay_cycles; i++) {
            __nop();
        }
        MAP_GPIO_setOutputLowOnPin(BUZZER_PIN);
        // Delay for the other half of the period
        for (i = 0; i < delay_cycles; i++) {
            __nop();
        }
    }
}

// Function to stop tone generation
void noTone(void) {
    // Simply return from the tone function to stop generating tone
    return;
}
