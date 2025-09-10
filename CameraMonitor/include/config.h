#ifndef CONFIG_H
#define CONFIG_H

// INCLUDES
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include "USB_STREAM.h"
#include "esp_heap_caps.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <SPI.h>
#include <SD.h>
#include "Audio.h"
#include <ESP32Servo.h>

// HARDWARE PINS & CAMERA SETTINGS
#define FRAME_WIDTH 800
#define FRAME_HEIGHT 600
#define FRAME_INTERVAL 333333
#define MJPEG_BUF_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 2)
#define USB_PAYLOAD_BUF_SIZE (32 * 1024)
#define USB_FRAME_BUF_SIZE (128 * 1024)

#define SD_CS     10
#define SPI_MOSI  11
#define SPI_MISO  13
#define SPI_SCK   12

#define I2S_DOUT  16 
#define I2S_BCLK  17
#define I2S_LRC   18

#define PIR_PIN   38

#define SERVO1_PIN 40
#define SERVO2_PIN 42

// AUDIO CONSTANTS
#define AUDIO_HELLO                0  
#define AUDIO_WIFI_FAILED          1  
#define AUDIO_WIFI_SUCCESS         2  
#define AUDIO_MOTION_DETECTED      3 

// SYSTEM CONSTANTS
#define MAX_CLIENTS 3
#define APP_CPU 1
#define PRO_CPU 0

// BLYNK VIRTUAL PINS
#define V_SERVO1_LEFT   V10
#define V_SERVO1_RIGHT  V11
#define V_SERVO2_DOWN   V12
#define V_SERVO2_UP     V13
#define V_SERVO_CENTER  V14

// ENUMS
enum WiFiState { 
    WIFI_STA_OK,   
    WIFI_AP_MODE    
};

#endif