#include "memory_monitor.h"

static unsigned long lastMemoryCheck = 0;
static uint32_t memoryCheckCount = 0;

void printMemoryStats() {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    
    Serial.println("========== MEMORY STATUS ==========");
    Serial.printf("[MEM] Free Heap: %d KB (%.1f%%)\n", 
                  freeHeap/1024, (float)freeHeap/totalHeap*100);
    Serial.printf("[MEM] Min Free: %d KB\n", minFreeHeap/1024);
    
    if (psramFound()) {
        uint32_t freePSRAM = ESP.getFreePsram();
        uint32_t totalPSRAM = ESP.getPsramSize();
        Serial.printf("[MEM] Free PSRAM: %d KB (%.1f%%)\n", 
                      freePSRAM/1024, (float)freePSRAM/totalPSRAM*100);
    }
    
    Serial.printf("[MEM] Uptime: %lu minutes\n", millis()/60000);
    Serial.println("==================================");
}

void checkMemoryHealth() {
    if (millis() - lastMemoryCheck < MEMORY_CHECK_INTERVAL) return;
    
    lastMemoryCheck = millis();
    memoryCheckCount++;
    
    uint32_t freeHeap = ESP.getFreeHeap();
    
    // Simple check - chỉ warning nếu < 50KB (rất thấp cho ESP32S3)
    if (freeHeap < 50000) {
        Serial.printf("[MEM] ⚠️ Low Heap: %d KB\n", freeHeap/1024);
    }
    
    // Periodic report mỗi 5 lần (2.5 phút)
    if (memoryCheckCount % 5 == 0) {
        Serial.printf("[MEM] Report #%d:\n", memoryCheckCount);
        printMemoryStats();
    }
}

void printBufferUsage() {
    Serial.println("========== BUFFER USAGE ==========");
    
    // Camera buffers
    size_t mjpegBufs = MJPEG_BUF_SIZE * 2;  // 2 buffers
    size_t usbBufs = USB_PAYLOAD_BUF_SIZE * 2 + USB_FRAME_BUF_SIZE;
    
    Serial.printf("[MEM] MJPEG Buffers: %d KB\n", mjpegBufs/1024);
    Serial.printf("[MEM] USB Buffers: %d KB\n", usbBufs/1024);
    Serial.printf("[MEM] Total Buffers: %d KB\n", (mjpegBufs + usbBufs)/1024);
    
    Serial.println("==================================");
}