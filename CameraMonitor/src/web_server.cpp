#include "wifi_manager.h"
#include "web_server.h"
#include "config.h"
#include "camera_handler.h"

WebServer server(80);
bool serverRunning = false;
bool apAdminLoggedIn = false;

QueueHandle_t clientQueue = NULL;
TaskHandle_t streamTaskHandle[MAX_CLIENTS] = {NULL, NULL, NULL};

void handle_root_mjpeg() {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32S3 Camera Stream - IUH</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            text-align: center; 
            background: linear-gradient(135deg, rgba(45, 74, 166, 0.9), rgba(30, 58, 138, 0.9));
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            color: #333;
            position: relative;
            overflow-x: hidden;
        }
        
        .container { 
            max-width: 900px; 
            margin: 0 auto; 
            background: rgba(255, 255, 255, 0.96);
            border-radius: 15px;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.3);
            overflow: hidden;
            backdrop-filter: blur(20px);
            position: relative;
            z-index: 1;
            border: 1px solid rgba(255, 255, 255, 0.3);
        }
        
        .header {
            background: linear-gradient(135deg, #2d4aa6, #1e3a8a);
            color: white;
            padding: 30px;
            position: relative;
            overflow: hidden;
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: 15px;
            right: 20px;
            width: 70px;
            height: 24px;
            background-image: url('https://icc.iuh.edu.vn/web/wp-content/uploads/2024/09/iuh_logo-chinh-thuc-1024x353.png');
            background-size: contain;
            background-repeat: no-repeat;
            background-position: center;
            opacity: 0.8;
            animation: pulse 4s ease-in-out infinite;
            filter: brightness(0) invert(1);
            z-index: 1;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 0.8; transform: scale(1); }
            50% { opacity: 0.95; transform: scale(1.05); }
        }
        
        .status { 
            color: #10b981; 
            font-weight: bold; 
            font-size: 18px;
            margin: 20px 0;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 20px 0;
            text-align: left;
        }
        
        .info-item {
            background: rgba(248, 250, 252, 0.9);
            padding: 15px;
            border-radius: 10px;
            border-left: 4px solid #2d4aa6;
            backdrop-filter: blur(5px);
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        
        .stream-container {
            padding: 30px;
            position: relative;
        }
        
        img { 
            max-width: 100%; 
            height: auto; 
            border: 3px solid #2d4aa6;
            border-radius: 10px;
            box-shadow: 0 15px 35px rgba(45, 74, 166, 0.3);
        }
        
        .btn {
            display: inline-block;
            padding: 12px 24px;
            background: linear-gradient(135deg, #2d4aa6, #1e3a8a);
            color: white;
            text-decoration: none;
            border-radius: 8px;
            margin: 10px;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(45, 74, 166, 0.3);
        }
        
        .btn:hover {
            background: linear-gradient(135deg, #1e3a8a, #1e40af);
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(45, 74, 166, 0.4);
        }
        
        .controls {
            margin: 20px 0;
            padding: 20px;
            background: rgba(241, 245, 249, 0.9);
            border-radius: 10px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(45, 74, 166, 0.1);
        }
        
        .university-info {
            background: linear-gradient(135deg, rgba(45, 74, 166, 0.1), rgba(30, 58, 138, 0.05));
            padding: 20px;
            border-radius: 12px;
            margin: 20px 0;
            border: 1px solid rgba(45, 74, 166, 0.2);
            backdrop-filter: blur(10px);
            position: relative;
        }
        
        .university-info::before {
            content: '';
            position: absolute;
            top: 15px;
            right: 15px;
            width: 50px;
            height: 17px;
            background-image: url('https://icc.iuh.edu.vn/web/wp-content/uploads/2024/09/iuh_logo-chinh-thuc-1024x353.png');
            background-size: contain;
            background-repeat: no-repeat;
            background-position: center;
            opacity: 0.4;
        }
        
        .university-info h3 {
            color: #2d4aa6;
            margin: 0 0 15px 0;
            font-size: 18px;
            font-weight: 700;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        
        .university-info p {
            margin: 8px 0;
            font-size: 14px;
            color: #1e3a8a;
            font-weight: 500;
        }
        
        .powered-by {
            margin-top: 30px;
            padding: 15px;
            background: rgba(45, 74, 166, 0.05);
            border-radius: 8px;
            text-align: center;
            border: 1px solid rgba(45, 74, 166, 0.1);
            position: relative;
        }
        
        .powered-by img {
            height: 25px;
            width: auto;
            opacity: 0.8;
            border: none;
            box-shadow: none;
            position: relative;
            z-index: 1;
        }
        
        @media (max-width: 768px) {
            .container {
                margin: 10px;
                border-radius: 10px;
            }
            .header {
                padding: 25px 20px;
            }
            .stream-container {
                padding: 20px;
            }
        }
        
        @media (max-width: 480px) {
            .container {
                margin: 5px;
            }
            .info-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>ESP32S3 Camera Stream</h1>
            <div class="status">Live Streaming Active</div>
        </div>
        
        <div class="stream-container">
            <div class="university-info">
                <h3>Industrial University of Ho Chi Minh City</h3>
                <p><strong>Project:</strong> ESP32S3 Camera Streaming System</p>
                <p><strong>Device:</strong> ESP32S3 N16R8 with Logitech C310</p>
                <p><strong>Department:</strong> Electronics & Computer Engineering</p>
                <p><strong>Developer:</strong> minhtri6090</p>
            </div>
            
            <div class="info-grid">
                <div class="info-item">
                    <strong>WiFi:</strong> %SSID%
                </div>
                <div class="info-item">
                    <strong>IP Address:</strong> %IP%
                </div>
                <div class="info-item">
                    <strong>Camera:</strong> Logitech C310
                </div>
                <div class="info-item">
                    <strong>Device:</strong> ESP32S3 N16R8
                </div>
            </div>
            
            <hr style="border: none; height: 1px; background: linear-gradient(90deg, transparent, #e2e8f0, transparent); margin: 30px 0;">
            
            <h2 style="color: #2d4aa6; text-shadow: 0 1px 2px rgba(0,0,0,0.1);">Live Camera Stream</h2>
            <img src="/stream" alt="Camera Stream" id="streamImg">
            
            <div class="controls">
                <a href="/stream" target="_blank" class="btn">Open in New Tab</a>
                <a href="javascript:location.reload()" class="btn">Refresh Page</a>
            </div>
            
            <div style="margin-top: 20px; font-size: 14px; color: #666;">
                <p><strong>Tips:</strong> If stream is slow, try refreshing or reduce number of viewers</p>
                <p><strong>Performance:</strong> Supports up to 3 concurrent viewers at ~15 FPS</p>
            </div>
            
            <div class="powered-by">
                <p style="margin: 0 0 10px 0; font-size: 12px; color: #666;">Powered by</p>
                <img src="https://icc.iuh.edu.vn/web/wp-content/uploads/2024/09/iuh_logo-chinh-thuc-1024x353.png" alt="IUH Logo"
                     onerror="this.style.display='none';">
            </div>
        </div>
    </div>
    
    <script>
        document.getElementById('streamImg').onerror = function() {
            console.log('Stream error, reloading...');
            setTimeout(() => {
                this.src = '/stream?' + new Date().getTime();
            }, 2000);
        };
        
        console.log('ESP32S3 Camera Stream Ready - IUH Project by minhtri6090!');
        console.log('WiFi: %SSID%');
        console.log('IP: %IP%');
    </script>
</body>
</html>
)=====";
    
    html.replace("%SSID%", WiFi.SSID());
    html.replace("%IP%", WiFi.localIP().toString());
    
    server.send(200, "text/html", html);
}

void stream_task(void *pvParameters) {
    stream_client_t* streamClient = (stream_client_t*)pvParameters;
    WiFiClient client = streamClient->client;
    Serial.printf("[TASK] Streaming client %s\n", client.remoteIP().toString().c_str());

    unsigned long lastFrameTime = 0;
    const unsigned long frameInterval = 66; // ~15 FPS

    while (streamClient->active && client.connected()) {
        if (millis() - lastFrameTime >= frameInterval) {
            uint8_t* frameBuffer = nullptr;
            size_t frameLen = 0;

            portENTER_CRITICAL(&frameMux);
            if (frame_ready_a && use_buf_a) {
                frameBuffer = mjpeg_buf_a;
                frameLen = frame_len_a;
                frame_ready_a = false;
                use_buf_a = false;
            } else if (frame_ready_b && !use_buf_a) {
                frameBuffer = mjpeg_buf_b;
                frameLen = frame_len_b;
                frame_ready_b = false;
                use_buf_a = true;
            }
            portEXIT_CRITICAL(&frameMux);

            if (frameBuffer && frameLen > 0) {
                String boundary = "--frame\r\n";
                boundary += "Content-Type: image/jpeg\r\n";
                boundary += "Content-Length: " + String(frameLen) + "\r\n\r\n";
                client.print(boundary);
                client.write(frameBuffer, frameLen);
                client.print("\r\n");
                lastFrameTime = millis();
            }
        }

        if (!client.connected()) break;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    Serial.printf("[TASK] Client %s disconnected, cleaning up\n", client.remoteIP().toString().c_str());
    client.stop();
    delete streamClient;

    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (streamTaskHandle[i] == current) {
            streamTaskHandle[i] = NULL;
            break;
        }
    }
    vTaskDelete(NULL);
}

void handle_stream() {
    Serial.println("[STREAM] Client requesting stream");
    start_stream_if_needed();

    WiFiClient client = server.client();
    if (!client.connected()) {
        Serial.println("[STREAM] Error: Invalid client");
        return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Cache-Control: no-cache, no-store, must-revalidate");
    client.println("Pragma: no-cache");
    client.println("Expires: 0");
    client.println("Connection: keep-alive");
    client.println();

    stream_client_t* streamClient = new stream_client_t;
    streamClient->client = client;
    streamClient->active = true;

    if (clientQueue != NULL) {
        if (xQueueSend(clientQueue, &streamClient, 0) != pdTRUE) {
            Serial.println("[STREAM] Max clients reached, rejecting");
            client.stop();
            delete streamClient;
        }
    }
}

void startMJPEGStreamingServer() {
    if (serverRunning) {
        Serial.println("[SERVER] Server already running");
        return;
    }
    
    clientQueue = xQueueCreate(MAX_CLIENTS, sizeof(stream_client_t*));
    if (clientQueue == NULL) {
        Serial.println("[SERVER] Failed to create client queue");
        return;
    }
    
    server.on("/", HTTP_GET, handle_root_mjpeg);
    server.on("/stream", HTTP_GET, handle_stream);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Not Found");
    });
    
    server.begin();
    serverRunning = true;
    
    Serial.println("[SERVER] MJPEG Streaming Server started");
    Serial.printf("[SERVER] Access: http://%s/\n", WiFi.localIP().toString().c_str());
    Serial.printf("[SERVER] Stream: http://%s/stream\n", WiFi.localIP().toString().c_str());
}

void stopMJPEGStreamingServer() {
    if (serverRunning) {
        server.stop();
        Serial.println("[SERVER] Streaming server stopped");
    }
    serverRunning = false;

    if (clientQueue != nullptr) {
        vQueueDelete(clientQueue);
        clientQueue = nullptr;
    }
}

void handleModernCSS() {
    String css = R"=====(
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
  background: linear-gradient(135deg, rgba(45, 74, 166, 0.9), rgba(30, 58, 138, 0.9));
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
  position: relative;
  overflow-x: hidden;
}

.container {
  background: rgba(255, 255, 255, 0.96);
  border-radius: 12px;
  box-shadow: 0 25px 50px -12px rgba(0,0,0,0.3);
  overflow: hidden;
  width: 100%;
  max-width: 400px;
  backdrop-filter: blur(20px);
  position: relative;
  z-index: 1;
  border: 1px solid rgba(255, 255, 255, 0.3);
}

.header {
  background: linear-gradient(135deg, #2d4aa6, #1e3a8a);
  color: white;
  padding: 30px 25px;
  text-align: center;
  position: relative;
  overflow: hidden;
}

.header h1 {
  font-size: 24px;
  font-weight: 700;
  margin-bottom: 8px;
  text-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.header p {
  opacity: 0.9;
  font-size: 14px;
  text-shadow: 0 1px 2px rgba(0,0,0,0.1);
}

.content {
  padding: 30px 25px;
  position: relative;
}

.form-group {
  margin-bottom: 20px;
}

.form-label {
  display: block;
  margin-bottom: 8px;
  font-weight: 600;
  color: #1e3a8a;
  font-size: 14px;
}

.form-input {
  width: 100%;
  padding: 12px 16px;
  border: 2px solid #cbd5e1;
  border-radius: 8px;
  font-size: 16px;
  transition: all 0.3s ease;
  background: rgba(255, 255, 255, 0.9);
}

.form-input:focus {
  outline: none;
  border-color: #2d4aa6;
  box-shadow: 0 0 0 3px rgba(45,74,166,0.1);
  background: white;
  transform: translateY(-1px);
}

.password-container {
  position: relative;
}

.password-container .form-input {
  padding-right: 45px;
}

.password-toggle {
  position: absolute;
  right: 12px;
  top: 50%;
  transform: translateY(-50%);
  background: none;
  border: none;
  cursor: pointer;
  padding: 4px;
  border-radius: 4px;
  transition: all 0.2s ease;
  color: #6b7280;
}

.password-toggle:hover {
  background: rgba(45, 74, 166, 0.1);
  color: #2d4aa6;
}

.password-toggle:focus {
  outline: none;
  box-shadow: 0 0 0 2px rgba(45, 74, 166, 0.2);
}

.eye-icon {
  width: 18px;
  height: 18px;
  display: inline-block;
}

.btn {
  display: block;
  width: 100%;
  padding: 12px 20px;
  border: none;
  border-radius: 8px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s ease;
  text-decoration: none;
  text-align: center;
  position: relative;
  overflow: hidden;
}

.btn::before {
  content: '';
  position: absolute;
  top: 0;
  left: -100%;
  width: 100%;
  height: 100%;
  background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
  transition: left 0.5s;
}

.btn:hover::before {
  left: 100%;
}

.btn-primary {
  background: linear-gradient(135deg, #2d4aa6, #1e3a8a);
  color: white;
  box-shadow: 0 4px 15px rgba(45, 74, 166, 0.3);
}

.btn-primary:hover {
  background: linear-gradient(135deg, #1e3a8a, #1e40af);
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(45, 74, 166, 0.4);
}

.btn-primary:disabled {
  background: #9ca3af;
  cursor: not-allowed;
  transform: none;
  box-shadow: none;
}

.btn-secondary {
  background: #f3f4f6;
  color: #1e3a8a;
  border: 2px solid #cbd5e1;
}

.btn-secondary:hover {
  background: #e5e7eb;
  border-color: #2d4aa6;
  transform: translateY(-1px);
}

.alert {
  padding: 12px 16px;
  border-radius: 8px;
  margin-bottom: 20px;
  position: relative;
  backdrop-filter: blur(5px);
}

.alert-error {
  background: rgba(254, 226, 226, 0.9);
  color: #dc2626;
  border: 1px solid #fca5a5;
}

.alert-info {
  background: rgba(45, 74, 166, 0.1);
  color: #1e3a8a;
  border: 1px solid rgba(45, 74, 166, 0.2);
}

.wifi-item {
  display: flex;
  align-items: center;
  padding: 12px;
  border: 2px solid #cbd5e1;
  border-radius: 8px;
  margin-bottom: 8px;
  cursor: pointer;
  transition: all 0.3s ease;
  background: rgba(249, 250, 251, 0.8);
  backdrop-filter: blur(5px);
}

.wifi-item:hover {
  border-color: #2d4aa6;
  background: rgba(255, 255, 255, 0.9);
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(45, 74, 166, 0.1);
}

.wifi-item.selected {
  border-color: #2d4aa6;
  background: rgba(45,74,166,0.05);
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(45, 74, 166, 0.2);
}

.wifi-name {
  flex: 1;
  font-weight: 500;
  margin-right: 12px;
  color: #1e3a8a;
}

.wifi-security {
  font-size: 12px;
  color: #6b7280;
  background: rgba(107, 114, 128, 0.1);
  padding: 2px 6px;
  border-radius: 4px;
}

.university-header {
  background: linear-gradient(135deg, rgba(45, 74, 166, 0.1), rgba(30, 58, 138, 0.05));
  padding: 12px 16px;
  margin: -30px -25px 20px -25px;
  border-bottom: 1px solid rgba(45, 74, 166, 0.2);
  text-align: center;
  position: relative;
}

.university-header h3 {
  color: #2d4aa6;
  font-size: 14px;
  margin: 0;
  font-weight: 600;
  text-shadow: 0 1px 2px rgba(0,0,0,0.05);
}

.loading {
  text-align: center;
  padding: 40px 20px;
  color: #6b7280;
}

.spinner {
  display: inline-block;
  width: 20px;
  height: 20px;
  border: 3px solid rgba(45, 74, 166, 0.1);
  border-top: 3px solid #2d4aa6;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-right: 10px;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

@media (max-width: 480px) {
  .container {
    margin: 10px;
    max-width: none;
  }
  
  .header {
    padding: 25px 20px;
  }
  
  .content {
    padding: 25px 20px;
  }
}
)=====";
    server.send(200, "text/css", css);
}

void handleModernRootAP() {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Config</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>WiFi Config</h1>
            <p>Configuration Portal</p>
        </div>
        
        <div class="content">
            <div class="university-header">
                <h3>Industrial University of Ho Chi Minh City</h3>
            </div>
            
            <form method="POST" action="/login">
                <div class="form-group">
                    <label class="form-label">Username</label>
                    <input type="text" name="username" class="form-input" placeholder="Enter username" required>
                </div>
                
                <div class="form-group">
                    <label class="form-label">Password</label>
                    <div class="password-container">
                        <input type="password" name="password" id="passwordInput" class="form-input" placeholder="Enter password" required>
                        <button type="button" class="password-toggle" onclick="togglePassword()">
                            <svg class="eye-icon" id="eyeIcon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </button>
                    </div>
                </div>
                
                <button type="submit" class="btn btn-primary">Login</button>
            </form>
        </div>
    </div>
    
    <script>
        function togglePassword() {
            const passwordInput = document.getElementById('passwordInput');
            const eyeIcon = document.getElementById('eyeIcon');
            
            if (passwordInput.type === 'password') {
                passwordInput.type = 'text';
                eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
            } else {
                passwordInput.type = 'password';
                eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
            }
        }
    </script>
</body>
</html>
)=====";
    server.send(200, "text/html; charset=utf-8", html);
}

void handleModernLoginAP() {
    bool loginSuccess = (
        server.hasArg("username") &&
        server.hasArg("password") &&
        server.arg("username") == "admin" && 
        server.arg("password") == "admin"
    );
    
    if (loginSuccess) {
        apAdminLoggedIn = true;
        server.sendHeader("Location", "/scan");
        server.send(302, "text/plain", "");
        return;
    }
    
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Failed</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Login Failed</h1>
            <p>Invalid credentials</p>
        </div>
        
        <div class="content">
            <div class="university-header">
                <h3>Industrial University of Ho Chi Minh City</h3>
            </div>
            
            <div class="alert alert-error">Wrong username or password!</div>
            
            <form method="POST" action="/login">
                <div class="form-group">
                    <label class="form-label">Username</label>
                    <input type="text" name="username" class="form-input" placeholder="Enter username" required autofocus>
                </div>
                
                <div class="form-group">
                    <label class="form-label">Password</label>
                    <div class="password-container">
                        <input type="password" name="password" id="passwordInput" class="form-input" placeholder="Enter password" required>
                        <button type="button" class="password-toggle" onclick="togglePassword()">
                            <svg class="eye-icon" id="eyeIcon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </button>
                    </div>
                </div>
                
                <button type="submit" class="btn btn-primary">Try Again</button>
            </form>
        </div>
    </div>
    
    <script>
        function togglePassword() {
            const passwordInput = document.getElementById('passwordInput');
            const eyeIcon = document.getElementById('eyeIcon');
            
            if (passwordInput.type === 'password') {
                passwordInput.type = 'text';
                eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
            } else {
                passwordInput.type = 'password';
                eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
            }
        }
    </script>
</body>
</html>
)=====";
    server.send(401, "text/html; charset=utf-8", html);
}

void handleModernScanAP() {
    if (!apAdminLoggedIn) {
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
        return;
    }

    if (server.method() == HTTP_POST) {
        String ssid = server.arg("ssid");
        String pass = server.arg("password");

        if (ssid.length() == 0) {
            server.send(400, "text/html", getErrorPage("Please select a WiFi network"));
            return;
        }

        Serial.printf("Received connection request: SSID='%s'\n", ssid.c_str());
        saveCredentials(ssid, pass);
        
        String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Connecting...</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Connecting...</h1>
            <p>Please wait</p>
        </div>
        <div class="content">
            <div class="university-header">
                <h3>Industrial University of Ho Chi Minh City</h3>
            </div>
            
            <div class="alert alert-info">
                Connecting to WiFi network. Device will automatically switch to camera mode when connected.
            </div>
            <p style="margin-top: 20px; color: #6b7280; text-align: center;">
                This page will automatically redirect when connection is established (30s timeout).
            </p>
        </div>
    </div>
    
    <script>
        setTimeout(() => {
            window.location.href = '/';
        }, 5000);
    </script>
</body>
</html>
)=====";

        server.send(200, "text/html", html);
        connecting = true;
        connectingSSID = ssid;
        connectingPassword = pass;
        connectStartTime = millis();
        return;
    }

    String loadingHtml = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Scanning WiFi Networks</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Scanning Networks</h1>
            <p>Please wait...</p>
        </div>
        <div class="content">
            <div class="university-header">
                <h3>Industrial University of Ho Chi Minh City</h3>
            </div>
            
            <div class="loading">
                <div class="spinner"></div>
                Scanning for available WiFi networks...
            </div>
            <div style="text-align: center; margin-top: 20px; color: #6b7280;">
                <p>This may take a few seconds</p>
            </div>
        </div>
    </div>
    
    <script>
        setTimeout(() => {
            window.location.href = '/scan-results';
        }, 3000);
    </script>
</body>
</html>
)=====";
    
    server.send(200, "text/html", loadingHtml);
}

void handleScanResults() {
    if (!apAdminLoggedIn) {
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
        return;
    }
    
    WiFi.mode(WIFI_AP_STA);
    delay(200);
    int n = WiFi.scanNetworks(false, true, false, 300);
    
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Select WiFi Network</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Select WiFi Network</h1>
            <p>Found %%COUNT%% networks</p>
        </div>
        
        <div class="content">
            <div class="university-header">
                <h3>Industrial University of Ho Chi Minh City</h3>
            </div>
            
            <form method="POST" action="/scan" id="wifiForm">
                <div class="form-group">
                    <label class="form-label">Available Networks</label>
                    <div style="max-height: 250px; overflow-y: auto; border: 1px solid #cbd5e1; border-radius: 8px; padding: 8px;">
                        %%WIFI_LIST%%
                    </div>
                </div>
                
                <div class="form-group">
                    <label class="form-label">WiFi Password</label>
                    <div class="password-container">
                        <input type="password" name="password" id="passwordInput" class="form-input" 
                               placeholder="Enter password (leave empty for open networks)" maxlength="63">
                        <button type="button" class="password-toggle" onclick="togglePassword()">
                            <svg class="eye-icon" id="eyeIcon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </button>
                    </div>
                </div>
                
                <button type="submit" class="btn btn-primary" id="connectBtn" disabled>
                    Connect to WiFi
                </button>
                
                <a href="/" class="btn btn-secondary" style="margin-top: 10px;">Back to Login</a>
            </form>
            
            <div style="margin-top: 20px; font-size: 14px; color: #6b7280;">
                <p><strong>Instructions:</strong></p>
                <p>1. Select a network from the list above</p>
                <p>2. Enter the correct WiFi password</p>
                <p>3. Wait for automatic connection and redirection</p>
            </div>
        </div>
    </div>

    <script>
        let selectedSSID = '';
        
        function selectWiFi(ssid, element) {
            selectedSSID = ssid;
            
            document.querySelectorAll('.wifi-item').forEach(item => {
                item.classList.remove('selected');
            });
            
            element.classList.add('selected');
            
            let ssidInput = document.getElementById('ssidInput');
            if (!ssidInput) {
                ssidInput = document.createElement('input');
                ssidInput.type = 'hidden';
                ssidInput.name = 'ssid';
                ssidInput.id = 'ssidInput';
                document.getElementById('wifiForm').appendChild(ssidInput);
            }
            ssidInput.value = ssid;
            
            const connectBtn = document.getElementById('connectBtn');
            connectBtn.disabled = false;
            connectBtn.style.opacity = '1';
            
            document.getElementById('passwordInput').focus();
        }
        
        function togglePassword() {
            const passwordInput = document.getElementById('passwordInput');
            const eyeIcon = document.getElementById('eyeIcon');
            
            if (passwordInput.type === 'password') {
                passwordInput.type = 'text';
                eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
            } else {
                passwordInput.type = 'password';
                eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
            }
        }
        
        document.getElementById('wifiForm').addEventListener('submit', function(e) {
            if (!selectedSSID) {
                e.preventDefault();
                alert('Please select a WiFi network first!');
                return false;
            }
            
            const connectBtn = document.getElementById('connectBtn');
            connectBtn.innerHTML = 'Connecting...';
            connectBtn.disabled = true;
        });
    </script>
</body>
</html>
)=====";

    String wifiList = "";
    if (n == 0) {
        wifiList = "<div class='alert alert-error'>No networks found. <button onclick='location.reload()' class='btn btn-secondary'>Scan Again</button></div>";
    } else {
        for (int i = 0; i < n; i++) {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            wifi_auth_mode_t encType = WiFi.encryptionType(i);
            
            if (ssid.length() == 0) continue;
            
            bool isOpen = (encType == WIFI_AUTH_OPEN);
            String security = isOpen ? "Open" : "Secured";
            
            String escapedSSID = ssid;
            escapedSSID.replace("\"", "&quot;");
            
            wifiList += "<div class='wifi-item' onclick='selectWiFi(\"" + escapedSSID + "\", this);' title='Signal: " + String(rssi) + " dBm'>";
            wifiList += "<span class='wifi-name'>" + ssid + "</span>";
            wifiList += "<span class='wifi-security'>" + security + "</span>";
            wifiList += "</div>";
        }
    }
    
    html.replace("%%COUNT%%", String(n));
    html.replace("%%WIFI_LIST%%", wifiList);
    
    server.send(200, "text/html", html);
    WiFi.mode(WIFI_AP);
}

String getErrorPage(String message) {
    return R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Error</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Error</h1>
            <p>Something went wrong</p>
        </div>
        <div class="content">
            <div class="university-header">
                <h3>Industrial University of Ho Chi Minh City</h3>
            </div>
            
            <div class="alert alert-error">)====" + message + R"=====(</div>
            <a href="/scan" class="btn btn-primary">Try Again</a>
            <a href="/" class="btn btn-secondary" style="margin-top: 10px;">Back to Home</a>
        </div>
    </div>
</body>
</html>
)=====";
}

void startModernAPWebServer() {
    apAdminLoggedIn = false;
    
    server.on("/", HTTP_GET, handleModernRootAP);
    server.on("/login", HTTP_POST, handleModernLoginAP);
    server.on("/scan", HTTP_GET, handleModernScanAP);
    server.on("/scan", HTTP_POST, handleModernScanAP);
    server.on("/scan-results", HTTP_GET, handleScanResults);
    server.on("/style.css", HTTP_GET, handleModernCSS);
    
    server.begin();
    serverRunning = true;
    Serial.printf("Camera Configuration Portal: http://%s/\n", WiFi.softAPIP().toString().c_str());
}

void startAPWebServer() { 
    startModernAPWebServer(); 
}

void handleRootAP() { 
    handleModernRootAP(); 
}

void handleLoginAP() { 
    handleModernLoginAP(); 
}  

void handleScanAP() { 
    handleModernScanAP(); 
}

void handleStyleCSS() { 
    handleModernCSS(); 
}

void handleWebServerLoop() {
    if (serverRunning) {
        server.handleClient();
    }
}

// ===============================================
// UTILITY FUNCTIONS
// ===============================================

String getSignalIcon(int rssi) { 
    return ""; 
}

String getConnectingPage(String ssid) { 
    return ""; 
}

String getAdvancedConnectingPage(String ssid) { 
    return ""; 
}

String getSuccessPage(String ip) { 
    return ""; 
}

void handleConnectionStatus() { 
    server.send(404, "text/plain", "Not Found"); 
}

void handleNotFound() {
    server.send(404, "text/plain", "Not Found");
}

void initWebServer() {
    server.onNotFound(handleNotFound);
}

void stopWebServer() {
    if (serverRunning) {
        server.stop();
        serverRunning = false;
    }
}

bool isWebServerRunning() {
    return serverRunning;
}

void restartWebServer() {
    stopWebServer();
    delay(1000);
    
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        startModernAPWebServer();
    } else if (WiFi.getMode() == WIFI_STA) {
        startMJPEGStreamingServer();
    }
}