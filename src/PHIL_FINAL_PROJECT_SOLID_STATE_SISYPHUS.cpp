// Ashwin Wahlquist
// PHIL 3009 Final Project

#include "Particle.h" // imports the Particle library; microcontroller being used is the "Particle Photon 2 (PP2)"

SYSTEM_MODE(MANUAL); // PP2 is a Wi-Fi capable device - Manual Mode stops the firmware from attempting to connect to the internet (it is unneeded!)

const int ledPin = D5; // define LED output pin (Pin D5) - (by declaring "const" or "constant", we ensure these values can never be accidentally changed in the future).
const int buttonPin = D4; // define button input register pin (Pin D4) - (by defining "int" or "integer", we know that variables ledPin and buttonPin are numerical).

int brightness = 255; // defines the brightness of the LED as a variable called "brightness" with initial value of 255 (full brightness).
const int gravity = 2; // defines how much brightness decreases every fade interval - (const because, like gravity, it doesn't change).
const int humanEffort = 25; // defines how much each button press adds to brightness (const because pushing the button harder or softer does not do anything).

unsigned long lastFadeTime = 0; // timestamp of the last moment gravity fired - starts at 0 because no time has passed on boot (unsigned long because time is always positive and grows very large).
const int fadeInterval = 50; // milliseconds that must pass between each gravity tick - (const because the pace of the universe never changes).

int buttonState; // stores the last confirmed stable state of the button (HIGH or LOW) after debouncing - uninitialized because we don't know the button state until the program starts.
int lastReading = HIGH; // stores the raw most recent button reading - initialized to HIGH because INPUT_PULLUP means the button reads HIGH by default when unpressed.
unsigned long lastDebounceTime = 0; // timestamp of the last moment the button reading changed - used to filter out electrical noise (bounce).
const int debounceDelay = 50; // milliseconds the button reading must be stable before we trust it - (const because the noise characteristics of the button never change).

unsigned long lastPWMTime = 0; // timestamp of the last PWM cycle - reserved for future use, currently unused in the loop logic.
const int pwmPeriod = 15; // total length of one software PWM cycle in milliseconds - shorter = smoother flicker, longer = more visible flicker.

void setup() { // runs once on boot before loop() begins
    pinMode(ledPin, OUTPUT); // configures D5 as an output pin so the PP2 can send voltage to the LED
    pinMode(buttonPin, INPUT_PULLUP); // configures D4 as an input pin with internal pull-up resistor - means button reads HIGH by default and LOW when pressed
    Serial.begin(9600); // opens the serial communication channel at 9600 baud so diagnostic messages can be read on a computer
}

void loop() { // runs forever, repeating as fast as the PP2 can execute it
    unsigned long currentMillis = millis(); // captures the current time in milliseconds since boot - snapshot used for all timing comparisons this iteration

    // 1. GRAVITY
    if (currentMillis - lastFadeTime >= fadeInterval) { // checks if 50ms has passed since gravity last fired
        if (brightness > 0) { // only applies gravity if the LED isn't already fully off - prevents brightness going below 0
            brightness -= gravity; // subtracts gravity (2) from brightness, dimming the LED slightly
            if (brightness < 0) brightness = 0; // safety clamp - prevents brightness from going negative due to overshooting
        }
        lastFadeTime = currentMillis; // resets the gravity timestamp so the next tick waits another 50ms
    }

    // 2. HUMAN STRUGGLE
    int reading = digitalRead(buttonPin); // reads the raw current state of the button pin (HIGH = unpressed, LOW = pressed)

    if (reading != lastReading) { // checks if the button state has changed since last loop iteration
        lastDebounceTime = currentMillis; // if it changed, reset the debounce timer - we won't trust this reading until it stays stable for 50ms
    }

    if ((currentMillis - lastDebounceTime) > debounceDelay) { // checks if the button reading has been stable for 50ms - if so, we trust it
        if (reading != buttonState) { // checks if the stable reading is different from the last confirmed state
            buttonState = reading; // updates the confirmed button state to the new stable reading

            if (buttonState == LOW) { // only acts on a press down, not a release - LOW means the button is being pressed
                brightness += humanEffort; // adds humanEffort (25) to brightness, pushing the LED brighter
                Serial.println("The human pushes the boulder. Effort registered."); // prints diagnostic message to serial monitor

                if (brightness >= 255) { // checks if the human has pushed brightness to maximum - the illusion of victory
                    brightness = 0; // instantly resets brightness to 0 (off) - the absurd punishment for winning
                    Serial.println("CRITICAL FAILURE: The universe refuses meaning. The boulder resets."); // prints the cruel outcome to serial monitor
                }
            }
        }
    }
    lastReading = reading; // stores this iteration's raw reading so next iteration can detect changes

    // 3. SOFTWARE PWM
    int pwmPosition = currentMillis % pwmPeriod; // calculates where we are within the current 15ms PWM cycle (0 to 14)
    int threshold = (brightness * pwmPeriod) / 255; // calculates how many milliseconds of the cycle the LED should be ON - higher brightness = longer ON time
    if (pwmPosition < threshold) { // if we are in the ON portion of the cycle
        digitalWrite(ledPin, HIGH); // turns the LED fully on
    } else { // if we are in the OFF portion of the cycle
        digitalWrite(ledPin, LOW); // turns the LED fully off
    }