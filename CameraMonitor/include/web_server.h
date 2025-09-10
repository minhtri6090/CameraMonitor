#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"

extern WebServer server;
extern bool serverRunning;

typedef struct {
    WiFiClient client;
    bool active;
} stream_client_t;

extern QueueHandle_t clientQueue;
extern TaskHandle_t streamTaskHandle[MAX_CLIENTS];

extern void stream_task(void *pvParameters);

void startMJPEGStreamingServer();
void stopMJPEGStreamingServer();
void handleWebServerLoop();

void handle_root_mjpeg();
void handle_stream();

void startModernAPWebServer();
void startAPWebServer();

void handleModernRootAP();
void handleModernLoginAP();
void handleModernScanAP();
void handleModernCSS();
void handleConnectionStatus();

void handleRootAP();
void handleLoginAP();
void handleScanAP();
void handleStyleCSS();

String getSignalIcon(int rssi);
String getConnectingPage(String ssid);
String getAdvancedConnectingPage(String ssid);
String getErrorPage(String message);
String getSuccessPage(String ip);

#endif