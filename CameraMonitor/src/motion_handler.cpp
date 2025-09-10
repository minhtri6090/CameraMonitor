#include "motion_handler.h"
#include "audio_handler.h"

bool systemReady = false;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long motionCooldown = 10000; 

int pirState = LOW; 
int pirVal = 0; 

void IRAM_ATTR motionISR() {
    if (systemReady && (millis() - lastMotionTime > motionCooldown)) {
        motionDetected = true;
    }
}

void initializePIR() {
    Serial.println("[PIR] Initializing PIR sensor...");

    pinMode(PIR_PIN, INPUT);

    detachInterrupt(digitalPinToInterrupt(PIR_PIN));
    
    Serial.printf("[PIR] PIR sensor configured on pin %d with PULLDOWN\n", PIR_PIN);

    Serial.println("[PIR] PIR warming up (5 seconds)...");
    for (int i = 5; i > 0; i--) {
        Serial.printf("[PIR] Warming up... %d seconds remaining\n", i);

        for (int j = 0; j < 10; j++) {
            yield();
            delay(100);
        }
    }

    pirVal = digitalRead(PIR_PIN);
    pirState = LOW;
    motionDetected = false;
    lastMotionTime = millis();
    
    Serial.printf("[PIR] Initial PIR reading: %d\n", pirVal);

    if (pirVal == LOW) {
        attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);
        Serial.println("[PIR] PIR sensor ready for motion detection");
    } else {
        Serial.println("[PIR] WARNING: PIR pin not stable (reading HIGH), interrupt not attached");
        Serial.println("[PIR] Check wiring or wait for sensor to stabilize");
    }
}

void handleMotionLoop() {
    if (systemReady) {
        pirVal = digitalRead(PIR_PIN);
        
        if (pirVal == HIGH) {
            if (pirState == LOW) {
                if (millis() - lastMotionTime > motionCooldown) {
                    Serial.println("[MOTION] Motion detected!");
                    lastMotionTime = millis();
                    pirState = HIGH;

                    if (!isAudioPlaying()) {
                        playAudio(AUDIO_MOTION_DETECTED);
                    }
                }
            }
        } else {
            if (pirState == HIGH) {
                Serial.println("[MOTION] Motion stopped");
                pirState = LOW;
            }
        }
    }

    if (motionDetected) {
        motionDetected = false;
        Serial.println("[MOTION] ISR Motion detected!");
        
        if (!isAudioPlaying()) {
            playAudio(AUDIO_MOTION_DETECTED);
        }
    }
}

void getPIRStatus() {
    if (systemReady) {
        int currentReading = digitalRead(PIR_PIN);
        Serial.printf("[PIR] Status - Pin: %d, State: %d, Ready: %s, Last motion: %lu ms ago\n", 
                     currentReading, pirState, systemReady ? "YES" : "NO", 
                     millis() - lastMotionTime);
    } else {
        Serial.println("[PIR] PIR sensor not ready");
    }
}