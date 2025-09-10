#ifndef MEMORY_MONITOR_H
#define MEMORY_MONITOR_H

#include "config.h"

// Simple memory monitoring for ESP32S3 N16R8
#define MEMORY_CHECK_INTERVAL 30000  // 30 seconds

void printMemoryStats();
void checkMemoryHealth();
void printBufferUsage();

#endif