/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdio.h>
#include <i2c_lcd.h>
#include <math.h>
#include <string.h>
// I2C LCD Library found https://github.com/hunterhedges-zz/I2cLcd

// define GPIO pins
#define SPEAKER_PIN GPIO_PORT_P2, GPIO_PIN5
#define LED_PIN GPIO_PORT_P2, GPIO_PIN1

//Address for LCD
#define LCD_ADDRESS 0x27

// max notes a person can play
#define MAX_NOTES 10

// tracking notes being played
uint16_t currentFreq[MAX_NOTES] = {0};
uint8_t currentVolume[MAX_NOTES] = {0};
uint8_t notesPlayed = 0;

typedef struct note_t {
    unsigned int frequency;
    unsigned int port;
    unsigned int pin;
    char* name;
}Note;

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
void playChord(void);
void addChordNote(uint16_t frequency, uint8_t volume);
void removeChordNote(uint16_t frequency);
char* findClosestNote(float frequency);
int findInterval(char* note1, char* note2);
char* determineChord(char* note1, char* note2, char* note3);



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
    addChordNote(A.frequency, 50); // A4 note at 50% volume
    addChordNote(523, 50); // C5 note at 50% volume
    addChordNote(587, 50); // D5 note at 50% volume
    __delay_cycles(3000000); // Delay for a while
    stopTone();

    // Remove a note chord
    printf("Remove note chord\n");
    removeChordNote(A.frequency); // removing A4 note
    __delay_cycles(3000000); // Delay for a while
    stopTone();

    // Example frequencies for a C major chord (C4, E4, G4)
    float frequencies[] = {261.63, 329.63, 392.00};
    int numFrequencies = sizeof(frequencies) / sizeof(frequencies[0]);

    char* notesPlayed[numFrequencies];
    for (int i = 0; i < numFrequencies; i++) {
        notesPlayed[i] = findClosestNote(frequencies[i]);
        printf("Frequency: %.2f Hz, Note: %s\n", frequencies[i], notesPlayed[i]);
    }

    char* chordName = determineChord(notesPlayed[0], notesPlayed[1], notesPlayed[2]);
    printf("Chord: %s\n", chordName);

    //LCD Testing
    LCD_cursorOn(); //doesn't work
    LCD_backlightOff(); //only command I tested that works, only tried writeChar and writeString

    while (1) {
        // Main loop
        //Simple test for button input (works)
        if(!MAP_GPIO_getInputPinValue(C.port, C.pin)) {
            printf("%s played\n", C.name);
        }
        else if(!MAP_GPIO_getInputPinValue(C_SHARP.port, C_SHARP.pin)) {
            printf("%s played\n", C_SHARP.name);
        }
        else if(!MAP_GPIO_getInputPinValue(D.port, D.pin)) {
            printf("%s played\n", D.name);
        }
        else if(!MAP_GPIO_getInputPinValue(E_FLAT.port, E_FLAT.pin)) {
            printf("%s played\n", E_FLAT.name);
        }
        else if(!MAP_GPIO_getInputPinValue(E.port, E.pin)) {
            printf("%s played\n", E.name);
        }
        else if(!MAP_GPIO_getInputPinValue(F.port, F.pin)) {
            printf("%s played\n", F.name);
        }
        else if(!MAP_GPIO_getInputPinValue(F_SHARP.port, F_SHARP.pin)) {
            printf("%s played\n", F_SHARP.name);
        }
        else if(!MAP_GPIO_getInputPinValue(G.port, G.pin)) {
            printf("%s played\n", G.name);
        }
        else if(!MAP_GPIO_getInputPinValue(G_SHARP.port, G_SHARP.pin)) {
            printf("%s played\n", G_SHARP.name);
        }
        else if(!MAP_GPIO_getInputPinValue(A.port, A.pin)) {
            printf("%s played\n", A.name);
        }
        else if(!MAP_GPIO_getInputPinValue(B.port, B.pin)) {
            printf("%s played\n", B.name);
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

void playChord() {
    if(notesPlayed == 0) {
        return;
    }

    uint32_t totalPeriod = 0;
    uint32_t totalDutyCycle = 0;

    uint16_t i = 0;     
    for(i = 0; i < notesPlayed; i++) {
        uint16_t period = 3000000 / currentFreq[i];
        uint16_t dutyCycle = (period * currentVolume[i]) / 100;

        totalPeriod += period;
        totalDutyCycle += dutyCycle;
    }

    // Software mixing example (basic)
    uint16_t mixedPeriod = totalPeriod / notesPlayed;
    uint16_t mixedDutyCycle = totalDutyCycle / notesPlayed;

    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, mixedPeriod - 1);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, mixedDutyCycle);
}

void addChordNote(uint16_t frequency, uint8_t volume) {
    if(notesPlayed < MAX_NOTES) {
        currentFreq[notesPlayed] = frequency;
        currentVolume[notesPlayed] = volume;
        notesPlayed++;
        playChord();
    }
    else {
        printf("Cannot add more notes");
    }
}

void removeChordNote(uint16_t frequency) {
    uint16_t i = 0; 
    for(i = 0; i < notesPlayed; i++) {
        if(currentFreq[i] == frequency) {
            uint16_t j = 0;
            for(j = i; j < notesPlayed - 1; j++) {
                currentFreq[j] = currentFreq[j + 1];
                currentVolume[j] = currentVolume[j + 1];
            }
            notesPlayed--;
            playChord();
            break;
        }
    }
    printf("Note not found");
}

// Function to find the closest note to a given frequency
char* findClosestNote(float frequency) {
    float minDiff = fabs(frequency - notes[0].frequency);
    int minIndex = 0;

    for (int i = 1; i < NUM_NOTES; i++) {
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
    for (int i = 0; i < NUM_NOTES; i++) {
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

    for (int i = 0; i < NUM_CHORDS; i++) {
        if ((intervals[0] == chords[i].intervals[1] && intervals[1] == chords[i].intervals[2]) ||
            (intervals[0] == chords[i].intervals[2] && intervals[1] == chords[i].intervals[1])) {
            return chords[i].name;
        }
    }
    return "Unknown";
}
