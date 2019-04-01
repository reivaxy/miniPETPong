
#include "miniPETPong.h"
#include "initPageHtml.h"


int scl = D4;
int sda = D3;

SSD1306 display(0x3c, sda, scl);
Config config(CONFIG_VERSION, "miniPETPong", "miniPETPong");
static WiFiEventHandler wifiSTAGotIpHandler, wifiSTADisconnectedHandler,
                        stationConnectedHandler, stationDisconnectedHandler ;
ESP8266WebServer* server;
char otaMessage[50];
boolean waitingOta = false;
boolean connectedHomeSsid = false;
String ipOnHomeSsid;
char ipOnAP[16];

#define LEFT_BUTTON D6
#define RIGHT_BUTTON D5
#define FIRE_BUTTON D0

#define UP_BUTTON RIGHT_BUTTON
#define DOWN_BUTTON LEFT_BUTTON
const unsigned long PADDLE_RATE = 33;
const unsigned long BALL_RATE = 16;
const uint8_t PADDLE_HEIGHT = 24;

void drawCourt();

uint8_t ball_x = 64, ball_y = 32;
uint8_t ball_dir_x = 1, ball_dir_y = 1;
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 12;
uint8_t cpu_y = 16;

const uint8_t PLAYER_X = 115;
uint8_t player_y = 16;

boolean gameStarted = false;

void setup() {
  Serial.begin(115200);
  Serial.println();  
  Serial.println("Setup");
  
  pinMode(LEFT_BUTTON, INPUT); 
  pinMode(RIGHT_BUTTON, INPUT);  
  pinMode(FIRE_BUTTON, INPUT);

  config.init(); // Reads config from EEPROM or initializes it.

  display.init();
  display.setBrightness(250);
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  // Open wifi access point
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config.getAPSsid(), config.getAPPwd());
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected([&](const WiFiEventSoftAPModeStationConnected& evt){
    Serial.println("Station connected");
  });
  server = new ESP8266WebServer(80);
  server->on("/init", HTTP_GET, [](){
    Serial.println("Request on /init");
    printHomePage();
  });
  server->on("/ota", HTTP_GET, [](){   // would be better with post but hell
    gameStarted = false;
    ArduinoOTA.begin();  
    Serial.println("Request on /ota");
    strcpy(otaMessage, "Ready for OTA");
    ArduinoOTA.onStart([&]() {
      strcpy(otaMessage, "OTA started");
    });  
    ArduinoOTA.onEnd([&]() {
      strcpy(otaMessage, "OTA ended, Reboot");
      waitingOta = false;
    });  
    ArduinoOTA.onProgress([&](unsigned int progress, unsigned int total) {
      sprintf(otaMessage, "Progress: %u%%", (progress / (total / 100)));
      refreshMenu();
    });
    ArduinoOTA.onError([&](ota_error_t error) {
      sprintf(otaMessage, "OTA Error[%u]: ", error);      
      waitingOta = false;
    });
    waitingOta = true;
    char message[100];
    sprintf(message, "Upload using network port %s in Arduino IDE", ipOnHomeSsid.c_str());
    sendText(message, 200);
  });

  server->begin();
  // If home ssid was configured, try connecting to it.
  if(*config.getHomeSsid() != 0) {
    Serial.printf("Connecting to %s\n", config.getHomeSsid()) ;
    WiFi.begin(config.getHomeSsid(), config.getHomePwd()); 
    wifiSTAGotIpHandler = WiFi.onStationModeGotIP(onSTAGotIP); 
    wifiSTADisconnectedHandler = WiFi.onStationModeDisconnected(onSTADisconnected);  
  } 
  IPAddress ipAddress = WiFi.softAPIP();
  ipAddress.toString().getBytes((byte *)ipOnAP, 15);
  ipOnAP[15] = 0; 
  
}


void startGame() {
  unsigned long start = millis();
  display.clear();
  ball_update = millis();
  paddle_update = ball_update;
  gameStarted = true;
  drawCourt();
}

void loop() {
  // Check if any request to serve
  server->handleClient();
  if(!gameStarted) {
    if(waitingOta) {
      ArduinoOTA.handle();
    }
    refreshMenu();
  } else {
    refreshGame();
  }
}

void refreshGame() {
    //display.clear();

    bool update = false;
    unsigned long time = millis();

    static bool up_state = false;
    static bool down_state = false;

    up_state |= (digitalRead(UP_BUTTON) == LOW);
    down_state |= (digitalRead(DOWN_BUTTON) == LOW);

    if(time > ball_update) {
        uint8_t new_x = ball_x + ball_dir_x;
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if(new_x == 0 || new_x == 127) {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the horizontal walls.
        if(new_y == 0 || new_y == 63) {
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT) {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if(new_x == PLAYER_X
           && new_y >= player_y
           && new_y <= player_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // display.drawPixel(ball_x, ball_y, BLACK);
        // display.drawPixel(new_x, new_y, WHITE);
        display.setColor(BLACK);
        display.setPixel(ball_x, ball_y);
        display.setColor(WHITE);
        display.setPixel(new_x, new_y);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update = true;
    }

    if(time > paddle_update) {
        paddle_update += PADDLE_RATE;

        // CPU paddle
        display.setColor(BLACK);
        display.drawVerticalLine(CPU_X, cpu_y, PADDLE_HEIGHT);
        const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
        if(cpu_y + half_paddle > ball_y) {
            cpu_y -= 1;
        }
        if(cpu_y + half_paddle < ball_y) {
            cpu_y += 1;
        }
        if(cpu_y < 1) cpu_y = 1;
        if(cpu_y + PADDLE_HEIGHT > 63) cpu_y = 63 - PADDLE_HEIGHT;
        display.setColor(WHITE);
        display.drawVerticalLine(CPU_X, cpu_y, PADDLE_HEIGHT);

        // Player paddle
        display.setColor(BLACK);
        display.drawVerticalLine(PLAYER_X, player_y, PADDLE_HEIGHT);
        if(up_state) {
            player_y -= 1;
        }
        if(down_state) {
            player_y += 1;
        }
        up_state = down_state = false;
        if(player_y < 1) player_y = 1;
        if(player_y + PADDLE_HEIGHT > 63) player_y = 63 - PADDLE_HEIGHT;
        display.setColor(WHITE);
        display.drawVerticalLine(PLAYER_X, player_y, PADDLE_HEIGHT);

        update = true;
    }
    display.display();
}

void drawCourt() {
    display.drawRect(0, 0, 128, 64);
}

void refreshMenu() {
  display.clear();
  //display.drawString(0, 54, String(millis()));
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(127, 0, "PONG");  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  char message[100];
  if(waitingOta) {
    display.drawString(0, 15, otaMessage);  
  } else {
    display.drawString(0, 15, "Press fire to start");
  } 
  
  sprintf(message, "SSID: %s", config.getAPSsid());
  display.drawString(0, 30, message);  
  sprintf(message, "IP: %s", ipOnAP);
  display.drawString(0, 40, message);

  if(connectedHomeSsid) {
    sprintf(message, "Home: %s", ipOnHomeSsid.c_str());
    display.drawString(0, 50, message);
  }

  
  if(digitalRead(FIRE_BUTTON) == HIGH) {
    startGame();
  }
  display.display();
}

void printHomePage() {
  sendHtml(initPage, 200);
  server->on("/initSave",  HTTP_POST, [](){
    Serial.println("Rq on /initSave");
    // Read and save new AP SSID 
    String APSsid = server->arg("APSsid");
    if (APSsid.length() > 0) {
      // TODO: add checks
      config.setAPSsid(APSsid.c_str());
    }          
    // Read and save new AP PWD 
    String APPwd = server->arg("APPwd");
    if (APPwd.length() > 7) {
      // TODO: add checks
      config.setAPPwd(APPwd.c_str());
    }
    
    String homeSsid = server->arg("homeSsid");
    if (homeSsid.length() > 0) {
      // TODO: add checks
      config.setHomeSsid(homeSsid.c_str());
    }          
    // Read and save new AP PWD 
    String homePwd = server->arg("homePwd");
    if (homePwd.length() > 7) {
      // TODO: add checks
      config.setHomePwd(homePwd.c_str());
    }
    config.saveToEeprom();
    sendText("Please reboot", 200);
  });
}

void sendText(const char* text, int code) {
  server->sendHeader("Connection", "close");
  server->send(code, "text/plain", text);
}
void sendHtml(const char* html, int code) {
  server->sendHeader("Connection", "close");
  server->send(code, "text/html", html);
} 
// Called when STA is connected to home wifi and IP was obtained
void onSTAGotIP (WiFiEventStationModeGotIP ipInfo) {
  ipOnHomeSsid = ipInfo.ip.toString();
  Serial.printf("Got IP on %s: %s\n", config.getHomeSsid(), ipOnHomeSsid.c_str());
  connectedHomeSsid = true;
} 
void onSTADisconnected(const WiFiEventStationModeDisconnected& evt) {
  Serial.printf("Disconnected from %s\n", config.getHomeSsid());
}
