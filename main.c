/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdio.h>
#include <i2c_lcd.h>
// I2C LCD Library found https://github.com/hunterhedges-zz/I2cLcd

// define GPIO pins
#define SPEAKER_PIN GPIO_PORT_P2, GPIO_PIN5
#define LED_PIN GPIO_PORT_P2, GPIO_PIN1

//Address for LCD
#define LCD_ADDRESS 0x27


typedef struct note_t {
    unsigned int frequency;
    unsigned int port;
    unsigned int pin;
    char* name;
}Note;

//Notes for Middle Frequency
Note C = {262, GPIO_PORT_P3, GPIO_PIN7, "C"};
Note C_SHARP = {277, GPIO_PORT_P3, GPIO_PIN5, "C#"};
Note D = {294, GPIO_PORT_P5, GPIO_PIN1, "D"};
Note E_FLAT = {311, GPIO_PORT_P2, GPIO_PIN3, "Eb"};
Note E = {330, GPIO_PORT_P6, GPIO_PIN7, "E"};
Note F = {349, GPIO_PORT_P6, GPIO_PIN6, "F"};
Note F_SHARP = {370, GPIO_PORT_P5, GPIO_PIN6, "F#"};
Note G = {392, GPIO_PORT_P2, GPIO_PIN4, "G"};
Note G_SHARP = {415, GPIO_PORT_P2, GPIO_PIN6, "G#"};
Note A = {440, GPIO_PORT_P2, GPIO_PIN7, "A"};
Note B_FLAT = {466, GPIO_PORT_P3, GPIO_PIN6, "Bb"};
Note B = {493, GPIO_PORT_P5, GPIO_PIN2, "B"};

void initPWM(void);
void playTone(uint16_t frequency, uint8_t volume);
void stopTone(void);
void playChord(uint16_t freq1, uint16_t freq2, uint8_t volume);


int main(void) {
    WDT_A_holdTimer();

    initPWM();
    LCD_init(LCD_ADDRESS);

    // initialize GPIO input pins
    MAP_GPIO_setAsInputPinWithPullUpResistor(C.port, C.pin); // C pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(C_SHARP.port, C_SHARP.pin); // C_SHARP pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(D.port, D.pin); // D pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(E_FLAT.port, E_FLAT.pin); // E_FLAT pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(E.port, E.pin); // E pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(F.port, F.pin); // F pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(F_SHARP.port, F_SHARP.pin); // F_SHARP pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(G.port, G.pin); // G pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(G_SHARP.port, G_SHARP.pin); //G_SHARP pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(A.port, A.pin); // A pin
    MAP_GPIO_setAsInputPinWithPullUpResistor(B.port, B.pin); // B pin

    // Play a note
    //Simple test for speaker
    printf("\nPlay note\n");
    playTone(A.frequency, 50); // A4 note at 50% volume
    __delay_cycles(3000000); // Delay for a while
    stopTone();

    // Play a chord
    printf("Play chord\n");
    playChord(A.frequency, 523, 50); // A4 and C5 notes at 50% volume
    __delay_cycles(3000000); // Delay for a while
    stopTone();

    //LCD Testing
    LCD_cursorOn(); //doesn't work
    LCD_backlightOff(); //only command I tested that works, only tried writeChar and writeString

    while (1) {
        // Main loop
        //Simple test for button input (works)
        if(!MAP_GPIO_getInputPinValue(C.port, C.pin)) {
            printf("%s played\n", C.name);
        }
        if(!MAP_GPIO_getInputPinValue(C_SHARP.port, C_SHARP.pin)) {
            printf("%s played\n", C_SHARP.name);
        }
    }
}

void initPWM() {
    // Configure P2.7 as output for PWM
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);

    // Configure Timer_A for PWM mode
    Timer_A_PWMConfig pwmConfig = {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_1,
        3000, // Initial period (frequency)
        TIMER_A_CAPTURECOMPARE_REGISTER_4,
        TIMER_A_OUTPUTMODE_RESET_SET,
        1500  // Initial duty cycle (volume)
    };
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
}

void playTone(uint16_t frequency, uint8_t volume) {
    uint16_t period = 3000000 / frequency; // SMCLK is 3 MHz
    uint16_t dutyCycle = (period * volume) / 100;

    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, period - 1);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, dutyCycle);
}

void stopTone() {
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, 0);
}

void playChord(uint16_t freq1, uint16_t freq2, uint8_t volume) {
    uint16_t period1 = 3000000 / freq1;
    uint16_t period2 = 3000000 / freq2;

    uint16_t dutyCycle1 = (period1 * volume) / 100;
    uint16_t dutyCycle2 = (period2 * volume) / 100;

    // Software mixing example (basic)
    uint16_t mixedPeriod = (period1 + period2) / 2;
    uint16_t mixedDutyCycle = (dutyCycle1 + dutyCycle2) / 2;

    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, mixedPeriod - 1);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, mixedDutyCycle);
}
/*************** Old code, didn't want to delete yet *************/
//
//
//
//// tone functions
//void tone(uint32_t frequency);
//void noTone(void);
//
//void main(void) {
//    /* Stop Watchdog */
//    MAP_WDT_A_holdTimer();
//
//    // initialize GPIO
//    MAP_GPIO_setAsOutputPin(LED_PIN);
//    MAP_GPIO_setOutputLowOnPin(LED_PIN);
//
//
//    // initialize GPIO input pins
//    MAP_GPIO_setAsInputPinWithPullUpResistor(C.port, C.pin); // C pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(C_SHARP.port, C_SHARP.pin); // C_SHARP pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(D.port, D.pin); // D pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(E_FLAT.port, E_FLAT.pin); // E_FLAT pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(E.port, E.pin); // E pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(F.port, F.pin); // F pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(F_SHARP.port, F_SHARP.pin); // F_SHARP pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(G.port, G.pin); // G pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(G_SHARP.port, G_SHARP.pin); //G_SHARP pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(A.port, A.pin); // A pin
//    MAP_GPIO_setAsInputPinWithPullUpResistor(B.port, B.pin); // B pin
//
//    while (1)
//    {
//        // T_C pin
//        if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0))
//        {
//            tone(T_C);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN1))
//        {
//            // T_D pin
//            tone(T_D);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2))
//        {
//            // T_E pin
//            tone(T_E);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3))
//        {
//            // T_F pin
//            tone(T_F);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN4))
//        {
//            // T_G pin
//            tone(T_G);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5))
//        {
//            // T_A pin
//            tone(T_A);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else if (!MAP_GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6))
//        {
//            // T_B pin
//            tone(T_B);
//            MAP_GPIO_setOutputHighOnPin(LED_PIN);
//        }
//        else
//        {
//            // return
//            noTone();
//            MAP_GPIO_setOutputLowOnPin(LED_PIN);
//        }
//    }
//}
//
//// Function to generate a tone of a given frequency (in Hz)
//void tone(uint32_t frequency) {
//    // Calculate half-period (in microseconds)
//    uint32_t half_period_us = 500000 / frequency;
//
//    // Toggle buzzer pin
//    while (1) {
//        MAP_GPIO_setOutputHighOnPin(BUZZER_PIN);
//        // Delay for half the period
//        uint32_t delay_cycles = half_period_us * (SystemCoreClock / 1000000);
//        uint32_t i = 0;
//        for (i = 0; i < delay_cycles; i++) {
//            __nop();
//        }
//        MAP_GPIO_setOutputLowOnPin(BUZZER_PIN);
//        // Delay for the other half of the period
//        for (i = 0; i < delay_cycles; i++) {
//            __nop();
//        }
//    }
//}
//
//// Function to stop tone generation
//void noTone(void) {
//    // Simply return from the tone function to stop generating tone
//    return;
//}
