/*
 * Maxwell - Phase 4: Charging Dock ("Supercharger")
 *
 * This is the full car code (Phase 3) PLUS charging dock awareness.
 * When the car is docked and charging, it:
 *   - Detects voltage on the charging pin
 *   - Shows charging status on the web interface
 *   - Disables motors while docked (safety)
 *   - Shows battery level estimate
 *
 * The TP4056 module handles actual charging automatically.
 * This code just monitors the status and shows it on screen.
 *
 * Wiring additions (on top of Phase 3):
 *   TP4056 BAT+   --> 18650 Battery (+)
 *   TP4056 BAT-   --> 18650 Battery (-)
 *   TP4056 IN+    --> Copper pad (+) on car rear
 *   TP4056 IN-    --> Copper pad (-) on car rear
 *   TP4056 STDBY  --> ESP32 GPIO 12 (fully charged signal)
 *   TP4056 CHRG   --> ESP32 GPIO 16 (charging signal)
 *
 *   Dock pogo pins connect to copper pads when car drives in.
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// ========== CHANGE THESE TO YOUR WIFI ==========
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
// ================================================

// Motor pins
#define IN1 14
#define IN2 15
#define IN3 13
#define IN4 2

// Charging status pins (active LOW on TP4056)
#define PIN_CHARGING 16   // TP4056 CHRG pin — LOW when charging
#define PIN_STANDBY  12   // TP4056 STDBY pin — LOW when fully charged

// ESP32-CAM (AI Thinker) camera pins
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

bool isCharging = false;
bool isFullyCharged = false;

// ==================== MOTOR FUNCTIONS ====================

void moveForward()  { if (!isCharging) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  } }
void moveBackward() { if (!isCharging) { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); } }
void turnLeft()     { if (!isCharging) { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  } }
void turnRight()    { if (!isCharging) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); } }
void stopMotors()   { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  }

// ==================== CHARGING STATUS ====================

void updateChargingStatus() {
  // TP4056 pins are active LOW
  isCharging     = (digitalRead(PIN_CHARGING) == LOW);
  isFullyCharged = (digitalRead(PIN_STANDBY)  == LOW);

  // Safety: stop motors if docked/charging
  if (isCharging || isFullyCharged) {
    stopMotors();
  }
}

// ==================== WEB PAGE ====================

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Maxwell</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      background: #1a1a2e;
      color: #eee;
      font-family: 'Segoe UI', Arial, sans-serif;
      display: flex;
      flex-direction: column;
      align-items: center;
      height: 100vh;
      overflow: hidden;
      -webkit-user-select: none;
      user-select: none;
    }
    h1 {
      font-size: 1.2em;
      padding: 8px;
      color: #00d4ff;
      text-shadow: 0 0 10px rgba(0,212,255,0.3);
    }
    #stream-container {
      position: relative;
      width: 100%;
      max-width: 640px;
      flex: 1;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 0;
    }
    #stream {
      width: 100%;
      height: 100%;
      object-fit: contain;
      border-radius: 8px;
      border: 2px solid #333;
    }
    #battery-bar {
      width: 90%;
      max-width: 300px;
      height: 24px;
      background: #16213e;
      border: 2px solid #0f3460;
      border-radius: 12px;
      margin: 6px 0;
      position: relative;
      overflow: hidden;
    }
    #battery-fill {
      height: 100%;
      border-radius: 10px;
      transition: width 0.5s, background 0.5s;
    }
    #battery-text {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      font-size: 0.75em;
      font-weight: bold;
      text-shadow: 0 0 4px rgba(0,0,0,0.8);
    }
    .charging { animation: pulse 1.5s infinite; }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }
    #controls {
      display: grid;
      grid-template-columns: 80px 80px 80px;
      grid-template-rows: 80px 80px 80px;
      gap: 6px;
      padding: 12px;
    }
    .btn {
      background: #16213e;
      border: 2px solid #0f3460;
      border-radius: 12px;
      color: #00d4ff;
      font-size: 28px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      transition: all 0.1s;
      -webkit-tap-highlight-color: transparent;
    }
    .btn:active, .btn.active {
      background: #0f3460;
      border-color: #00d4ff;
      box-shadow: 0 0 15px rgba(0,212,255,0.4);
      transform: scale(0.95);
    }
    .btn.disabled {
      opacity: 0.3;
      pointer-events: none;
    }
    .btn-fwd  { grid-column: 2; grid-row: 1; }
    .btn-left { grid-column: 1; grid-row: 2; }
    .btn-stop { grid-column: 2; grid-row: 2; font-size: 14px; font-weight: bold; }
    .btn-right{ grid-column: 3; grid-row: 2; }
    .btn-back { grid-column: 2; grid-row: 3; }
    #status {
      font-size: 0.8em;
      color: #666;
      padding: 4px;
    }
    #gamepad-status {
      font-size: 0.75em;
      color: #00d4ff;
      padding: 2px 8px;
    }
    #gamepad-status.disconnected { color: #666; }
    @media (min-width: 600px) {
      #controls { gap: 8px; }
      .btn { width: 90px; height: 90px; font-size: 32px; }
    }
  </style>
</head>
<body>
  <h1>Maxwell</h1>

  <div id="battery-bar">
    <div id="battery-fill"></div>
    <div id="battery-text"></div>
  </div>

  <div id="stream-container">
    <img id="stream" src="">
  </div>

  <div id="controls">
    <button class="btn btn-fwd"   data-dir="forward"  >&#9650;</button>
    <button class="btn btn-left"  data-dir="left"     >&#9664;</button>
    <button class="btn btn-stop"  data-dir="stop"     >STOP</button>
    <button class="btn btn-right" data-dir="right"    >&#9654;</button>
    <button class="btn btn-back"  data-dir="backward" >&#9660;</button>
  </div>

  <div id="status">Connecting...</div>
  <div id="gamepad-status" class="disconnected">PS5 Controller: Press any button to connect</div>

  <script>
    const streamImg = document.getElementById('stream');
    const status = document.getElementById('status');
    const batteryFill = document.getElementById('battery-fill');
    const batteryText = document.getElementById('battery-text');
    const batteryBar = document.getElementById('battery-bar');

    // Start camera stream
    window.addEventListener('load', () => {
      const host = window.location.hostname;
      streamImg.src = `http://${host}:81/stream`;
      status.textContent = 'Connected';
      pollBattery();
    });

    // Poll charging status every 3 seconds
    function pollBattery() {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          const btns = document.querySelectorAll('.btn');

          if (data.charging) {
            batteryFill.style.width = '100%';
            batteryFill.style.background = 'linear-gradient(90deg, #f39c12, #e74c3c)';
            batteryText.textContent = 'CHARGING...';
            batteryBar.classList.add('charging');
            btns.forEach(b => b.classList.add('disabled'));
          } else if (data.full) {
            batteryFill.style.width = '100%';
            batteryFill.style.background = 'linear-gradient(90deg, #2ecc71, #27ae60)';
            batteryText.textContent = 'FULLY CHARGED';
            batteryBar.classList.remove('charging');
            btns.forEach(b => b.classList.remove('disabled'));
          } else {
            batteryFill.style.width = '70%';
            batteryFill.style.background = 'linear-gradient(90deg, #00d4ff, #0f3460)';
            batteryText.textContent = 'READY';
            batteryBar.classList.remove('charging');
            btns.forEach(b => b.classList.remove('disabled'));
          }
        })
        .catch(() => {});
      setTimeout(pollBattery, 3000);
    }

    // Send command to car
    function sendCommand(dir) {
      fetch(`/control?dir=${dir}`)
        .then(r => { status.textContent = dir.toUpperCase(); })
        .catch(e => { status.textContent = 'Connection lost'; });
    }

    // Button events (mouse + touch)
    document.querySelectorAll('.btn').forEach(btn => {
      const dir = btn.dataset.dir;

      btn.addEventListener('mousedown', (e) => {
        e.preventDefault();
        btn.classList.add('active');
        sendCommand(dir);
      });
      btn.addEventListener('mouseup', (e) => {
        btn.classList.remove('active');
        if (dir !== 'stop') sendCommand('stop');
      });
      btn.addEventListener('mouseleave', (e) => {
        btn.classList.remove('active');
      });

      btn.addEventListener('touchstart', (e) => {
        e.preventDefault();
        btn.classList.add('active');
        sendCommand(dir);
      });
      btn.addEventListener('touchend', (e) => {
        e.preventDefault();
        btn.classList.remove('active');
        if (dir !== 'stop') sendCommand('stop');
      });
    });

    // Keyboard controls
    const keyMap = {
      ArrowUp: 'forward', ArrowDown: 'backward',
      ArrowLeft: 'left', ArrowRight: 'right',
      w: 'forward', s: 'backward',
      a: 'left', d: 'right',
      ' ': 'stop'
    };
    const pressed = new Set();

    document.addEventListener('keydown', (e) => {
      const dir = keyMap[e.key];
      if (dir && !pressed.has(e.key)) {
        pressed.add(e.key);
        sendCommand(dir);
        const btn = document.querySelector(`[data-dir="${dir}"]`);
        if (btn) btn.classList.add('active');
      }
    });
    document.addEventListener('keyup', (e) => {
      const dir = keyMap[e.key];
      if (dir) {
        pressed.delete(e.key);
        if (dir !== 'stop') sendCommand('stop');
        const btn = document.querySelector(`[data-dir="${dir}"]`);
        if (btn) btn.classList.remove('active');
      }
    });

    // PS5 Controller (Gamepad API)
    const gpStatus = document.getElementById('gamepad-status');
    let lastGamepadCmd = 'stop';
    const DEADZONE = 0.3;

    window.addEventListener('gamepadconnected', (e) => {
      gpStatus.textContent = `PS5 Controller: ${e.gamepad.id.substring(0, 30)}... Connected!`;
      gpStatus.classList.remove('disconnected');
    });
    window.addEventListener('gamepaddisconnected', () => {
      gpStatus.textContent = 'PS5 Controller: Disconnected';
      gpStatus.classList.add('disconnected');
      sendCommand('stop');
    });

    function pollGamepad() {
      const gamepads = navigator.getGamepads();
      if (!gamepads) { requestAnimationFrame(pollGamepad); return; }

      const gp = gamepads[0];
      if (!gp) { requestAnimationFrame(pollGamepad); return; }

      const lx = gp.axes[0];
      const ly = gp.axes[1];

      const dpadUp    = gp.buttons[12] && gp.buttons[12].pressed;
      const dpadDown  = gp.buttons[13] && gp.buttons[13].pressed;
      const dpadLeft  = gp.buttons[14] && gp.buttons[14].pressed;
      const dpadRight = gp.buttons[15] && gp.buttons[15].pressed;
      const crossBtn  = gp.buttons[0] && gp.buttons[0].pressed;

      let cmd = 'stop';
      if (crossBtn) {
        cmd = 'stop';
      } else if (dpadUp    || ly < -DEADZONE) {
        cmd = 'forward';
      } else if (dpadDown  || ly > DEADZONE) {
        cmd = 'backward';
      } else if (dpadLeft  || lx < -DEADZONE) {
        cmd = 'left';
      } else if (dpadRight || lx > DEADZONE) {
        cmd = 'right';
      }

      if (cmd !== lastGamepadCmd) {
        lastGamepadCmd = cmd;
        sendCommand(cmd);
        document.querySelectorAll('.btn').forEach(b => b.classList.remove('active'));
        const btn = document.querySelector(`[data-dir="${cmd}"]`);
        if (btn) btn.classList.add('active');
      }

      requestAnimationFrame(pollGamepad);
    }
    requestAnimationFrame(pollGamepad);
  </script>
</body>
</html>
)rawliteral";

// ==================== HTTP HANDLERS ====================

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t control_handler(httpd_req_t *req) {
  char buf[32];
  size_t buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1 && buf_len <= sizeof(buf)) {
    httpd_req_get_url_query_str(req, buf, buf_len);
    char dir[16];
    if (httpd_query_key_value(buf, "dir", dir, sizeof(dir)) == ESP_OK) {
      Serial.printf("Command: %s\n", dir);
      if      (strcmp(dir, "forward")  == 0) moveForward();
      else if (strcmp(dir, "backward") == 0) moveBackward();
      else if (strcmp(dir, "left")     == 0) turnLeft();
      else if (strcmp(dir, "right")    == 0) turnRight();
      else if (strcmp(dir, "stop")     == 0) stopMotors();
    }
  }
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "OK", 2);
}

// Battery/charging status endpoint
static esp_err_t status_handler(httpd_req_t *req) {
  updateChargingStatus();
  char json[64];
  snprintf(json, sizeof(json),
    "{\"charging\":%s,\"full\":%s}",
    isCharging ? "true" : "false",
    isFullyCharged ? "true" : "false"
  );
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, json, strlen(json));
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];

  static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=frame";
  static const char* STREAM_BOUNDARY = "\r\n--frame\r\n";
  static const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

  res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      break;
    }

    size_t hlen = snprintf(part_buf, 64, STREAM_PART, fb->len);
    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK)
      res = httpd_resp_send_chunk(req, part_buf, hlen);
    if (res == ESP_OK)
      res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);

    esp_camera_fb_return(fb);
    if (res != ESP_OK) break;
  }
  return res;
}

// ==================== SERVER SETUP ====================

void startControlServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri  = { .uri = "/",        .method = HTTP_GET, .handler = index_handler,   .user_ctx = NULL };
  httpd_uri_t ctrl_uri   = { .uri = "/control",  .method = HTTP_GET, .handler = control_handler, .user_ctx = NULL };
  httpd_uri_t status_uri = { .uri = "/status",   .method = HTTP_GET, .handler = status_handler,  .user_ctx = NULL };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &ctrl_uri);
    httpd_register_uri_handler(camera_httpd, &status_uri);
  }
}

void startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;

  httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

// ==================== SETUP & LOOP ====================

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Maxwell + Supercharger ===\n");

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopMotors();

  // Charging status pins (TP4056 outputs are active LOW)
  pinMode(PIN_CHARGING, INPUT_PULLUP);
  pinMode(PIN_STANDBY,  INPUT_PULLUP);

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count     = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init FAILED: 0x%x\n", err);
    return;
  }
  Serial.println("Camera OK!");

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!\n");

  // Start servers
  startControlServer();
  startStreamServer();

  Serial.println("==========================================");
  Serial.print("  Open in browser: http://");
  Serial.println(WiFi.localIP());
  Serial.println("==========================================");
  Serial.println("  Controls:");
  Serial.println("    PS5 Controller: Left stick / D-pad");
  Serial.println("    Keyboard: Arrow keys / WASD");
  Serial.println("    X button / Space = Stop");
  Serial.println("==========================================");
  Serial.println("  Charging Dock:");
  Serial.println("    Drive into dock -> charges automatically");
  Serial.println("    Motors disabled while charging (safety)");
  Serial.println("==========================================\n");
}

void loop() {
  updateChargingStatus();

  if (isCharging) {
    Serial.println("Charging...");
  } else if (isFullyCharged) {
    Serial.println("Fully charged!");
  }

  delay(5000);
}
