/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdio.h>
#include <string.h>
#include <i2c_lcd.h>
// I2C LCD Library found https://github.com/hunterhedges-zz/I2cLcd

// define GPIO pins
#define SPEAKER GPIO_PORT_P2, GPIO_PIN7
#define LED_PIN GPIO_PORT_P2, GPIO_PIN1

//Address for LCD
#define LCD_ADDRESS 0x27

// max notes a person can play
#define MAX_NOTES 12
#define NUM_NOTES 12
#define NUM_CHORDS 3

// tracking notes being played
uint16_t totalPeriod = 0;
uint16_t totalDutyCycle = 0;
uint8_t notesPlayed = 0;

typedef struct note_t {
    unsigned int frequency;
    unsigned int port;
    unsigned int pin;
    char* name;
    uint8_t state;
}Note;

//Notes for Middle Frequency
Note notes[MAX_NOTES] = {
    {262, GPIO_PORT_P3, GPIO_PIN7, "C", 0},  // 0
    {277, GPIO_PORT_P3, GPIO_PIN5, "C#", 0}, // 1
    {294, GPIO_PORT_P5, GPIO_PIN1, "D", 0},  // 2
    {311, GPIO_PORT_P2, GPIO_PIN3, "Eb", 0}, // 3
    {330, GPIO_PORT_P6, GPIO_PIN7, "E", 0},  // 4
    {349, GPIO_PORT_P6, GPIO_PIN6, "F", 0},  // 5
    {370, GPIO_PORT_P5, GPIO_PIN6, "F#", 0}, // 6
    {392, GPIO_PORT_P2, GPIO_PIN4, "G", 0},  // 7
    {415, GPIO_PORT_P2, GPIO_PIN6, "G#", 0}, // 8
    {440, GPIO_PORT_P2, GPIO_PIN5, "A", 0},  // 9
    {466, GPIO_PORT_P3, GPIO_PIN6, "Bb", 0}, // 10
    {493, GPIO_PORT_P5, GPIO_PIN2, "B", 0}   // 11
};

typedef struct {
    char *name;
    int intervals[3]; // Major and minor triads have 3 notes
} Chord;

// Define basic chord types
Chord chords[NUM_CHORDS] = {
    {"Major", {0, 4, 7}}, // Root, Major third, Perfect fifth
    {"Minor", {0, 3, 7}}, // Root, Minor third, Perfect fifth
    {"Diminished", {0, 3, 6}} // Root, Minor third, Diminished fifth
};

void initPWM(void);
void stopTone(void);
void addTone(uint8_t);
void removeTone(uint8_t);
void __play(uint16_t period, uint16_t dutyCycle);
char* findClosestNote(float frequency);
int findInterval(char* note1, char* note2);
char* determineChord(char* note1, char* note2, char* note3);

int main(void) {
    WDT_A_holdTimer();

    initPWM();
    LCD_init(LCD_ADDRESS);

    // initialize GPIO input pins and their interrupts
    int i;
    for(i = 0; i < MAX_NOTES; i++) {
        MAP_GPIO_setAsInputPinWithPullUpResistor(notes[i].port, notes[i].pin);
        MAP_GPIO_clearInterruptFlag(notes[i].port, notes[i].pin);
        MAP_GPIO_enableInterrupt(notes[i].port, notes[i].pin);
        MAP_GPIO_interruptEdgeSelect(notes[i].port, notes[i].pin, GPIO_HIGH_TO_LOW_TRANSITION);
    }

    // Configure interrupts for each note's GPIO pin
    MAP_Interrupt_enableInterrupt(INT_PORT2);
    MAP_Interrupt_enableInterrupt(INT_PORT3);
    MAP_Interrupt_enableInterrupt(INT_PORT5);
    MAP_Interrupt_enableInterrupt(INT_PORT6);

    MAP_Interrupt_enableMaster();



    while (1) {
        MAP_PCM_gotoLPM0();
    }
}

/***** Helper Functions *****/
void initPWM() {
    // Configure P2.7 as output for PWM
    GPIO_setAsPeripheralModuleFunctionOutputPin(SPEAKER, GPIO_PRIMARY_MODULE_FUNCTION);

    // Configure Timer_A for PWM mode
    Timer_A_PWMConfig pwmConfig = {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_1,
        3000, // Initial period (frequency)
        TIMER_A_CAPTURECOMPARE_REGISTER_4,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0  // Initial duty cycle (50% volume)
    };
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
}

void stopTone() {
    Timer_A_stopTimer(TIMER_A0_BASE); // Stop the timer to stop the tone
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, 0); // Set duty cycle to 0
}

void addTone(uint8_t n) {
    uint16_t period = MAP_CS_getMCLK() / notes[n].frequency;
    notesPlayed++;
    totalPeriod += period;
    uint16_t dutyCycle = period / 2;
    totalDutyCycle += dutyCycle;

    stopTone();
    __play(totalPeriod / notesPlayed, totalDutyCycle / notesPlayed);
}

void __play(uint16_t period, uint16_t dutyCycle) {
    printf("Period: %d Duty Cycle: %d\n", period, dutyCycle);
    fflush(stdout);
    // Update Timer_A period and duty cycle
    Timer_A_stopTimer(TIMER_A0_BASE); // Stop the timer before configuring
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, period - 1);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, dutyCycle);
    Timer_A_clearTimer(TIMER_A0_BASE); // Clear the timer to reset the count
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE); // Start the timer in up mode
}

void removeTone(uint8_t n) {
    uint16_t period = MAP_CS_getMCLK() / notes[n].frequency;
    if (notesPlayed != 0) {
        notesPlayed--;
        totalPeriod -= period;
        uint16_t dutyCycle = period / 2;
        totalDutyCycle -= dutyCycle;
    }

    stopTone();
    if (notesPlayed) {
        __play(totalPeriod / notesPlayed, totalDutyCycle / notesPlayed);
    }

}

// Function to find the closest note to a given frequency
char* findClosestNote(float frequency) {
    float minDiff = fabs(frequency - notes[0].frequency);
    int minIndex = 0;

    int i;
    for (i = 1; i < NUM_NOTES; i++) {
        float diff = fabs(frequency - notes[i].frequency);
        if (diff < minDiff) {
            minDiff = diff;
            minIndex = i;
        }
    }

    return notes[minIndex].name;
}

// Function to find the interval between two notes
int findInterval(char* note1, char* note2) {
    int index1 = -1, index2 = -1;
    int i;
    for (i = 0; i < NUM_NOTES; i++) {
        if (strcmp(note1, notes[i].name) == 0) {
            index1 = i;
        }
        if (strcmp(note2, notes[i].name) == 0) {
            index2 = i;
        }
    }
    return (index2 - index1 + 12) % 12;
}

// Function to determine the chord from the notes
char* determineChord(char* note1, char* note2, char* note3) {
    int intervals[2];
    intervals[0] = findInterval(note1, note2);
    intervals[1] = findInterval(note1, note3);

    int i;
    for (i = 0; i < NUM_CHORDS; i++) {
        if ((intervals[0] == chords[i].intervals[1] && intervals[1] == chords[i].intervals[2]) ||
            (intervals[0] == chords[i].intervals[2] && intervals[1] == chords[i].intervals[1])) {
            return chords[i].name;
        }
    }
    return "Unknown";
}

/***** ISR Functions *****/
//ISR helper function since all serve the same purpose
// Interrupt Service Routine for Port 2, 3, 5, 6
void GPIO_ISR(void) {
    uint32_t status2 = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P2);
    uint32_t status3 = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);
    uint32_t status5 = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    uint32_t status6 = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P6);

    // Check each port and pin to see which one triggered the interrupt
    int i;
    for (i = 0; i < MAX_NOTES; i++) {
        if ((notes[i].port == GPIO_PORT_P2 && (status2 & notes[i].pin)) ||
            (notes[i].port == GPIO_PORT_P3 && (status3 & notes[i].pin)) ||
            (notes[i].port == GPIO_PORT_P5 && (status5 & notes[i].pin)) ||
            (notes[i].port == GPIO_PORT_P6 && (status6 & notes[i].pin))) {

            // Clear interrupt flag
            MAP_GPIO_clearInterruptFlag(notes[i].port, notes[i].pin);

            // Determine if button is pressed or released
            uint8_t currentButtonState = MAP_GPIO_getInputPinValue(notes[i].port, notes[i].pin);
            if (currentButtonState == GPIO_INPUT_PIN_LOW) {
                // Button pressed, play the note
                if (!notes[i].state) { // Only add if it was not already pressed
                    notes[i].state = 1;
                    addTone(i);
                    printf("%s played\n", notes[i].name);
                    LCD_writeString(notes[i].name, strlen(notes[i].name));
                    MAP_GPIO_interruptEdgeSelect(notes[i].port, notes[i].pin, GPIO_LOW_TO_HIGH_TRANSITION);
                }
            } else if (currentButtonState == GPIO_INPUT_PIN_HIGH) {
                // Button released, stop the note
                if (notes[i].state) { // Only remove if it was pressed
                    notes[i].state = 0;
                    removeTone(i);
                    LCD_clear();
                    LCD_home();
                    MAP_GPIO_interruptEdgeSelect(notes[i].port, notes[i].pin, GPIO_HIGH_TO_LOW_TRANSITION);
                }
            }
        }
    }
}

// ISR function for each port interrupt
void PORT2_IRQHandler(void) { GPIO_ISR(); }
void PORT3_IRQHandler(void) { GPIO_ISR(); }
void PORT5_IRQHandler(void) { GPIO_ISR(); }
void PORT6_IRQHandler(void) { GPIO_ISR(); }
