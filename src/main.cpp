/**
 * @file       main.cpp
 * @author     LSC Labs - P. Liebl
 * @version    1.0.2
 * @date       2025-12-05
 * @brief      Main application file
 * 
 * -> this file is a template from LSC Labs for IoT applications based on ESP8266/ESP32 (ESP-ProjectTemplate)
 * -> Modify for your own application as needed.
 * 
 * Template contains:
 * - WiFi management (Station + Access Point mode)
 * - MQTT client with Home Assistant auto discovery support
 * - Web Server with WebSocket support
 * 
 */
#include <Arduino.h>
#include <AppConfig.h>
#include <AppControl.h>
#include <FileSystem.h>
#include <Appl.h>
#include <WiFiController.h>
#include <MDNSController.h>
#include <CaptivePortal.h>
#include <MQTTController.h>
#include <WebSocket.h>
#include <WebServer.h>
// #include <WebRoutes.h>   - if you have custom web routes, include your WebRoutes.h here
#include <DevelopmentHelper.h>
#include <DHTSensor.h>
#include <AppControl.h>
#include <AppStatusLed.h>
#include <Button.h>
#include <WebRoutes.h>

// Instanziate the Sensor (Location 1, DHT22 on D4)
CDHTSensor oDHTSensor(D4,DHT22,1); // GPIO2 , DHT22 Sensor, Location 1 

CFS oFS;
CWiFiController oWiFiController;
CMDNSController oMDNSController; // Auto registers on Message Bus
// CCaptivePortal  oCaptivePortal;  // Auto registers on Message Bus
CMQTTController oMqttController;
CWebSocket      oWebSocket("/ws");
CWebServer      oWebServer(80);
CAppControl     oAppControl;
CAppStatusLed   oStatusLED(D6,D5,D0,false);
CButton         oBtn(D7,true);

// Insert an application control instance to handle the application coordination and logic
// CAppControl oAppControl;


#ifdef DEBUGINFOS
void runDebugTests() {
  JsonDocument oCfg;
  JsonObject oCfgNode = oCfg.to<JsonObject>();
  Appl.writeConfigTo(oCfgNode,false);
  DEBUG_INFO("Current configuration:");
  DEBUG_JSON_OBJ(oCfgNode);
}
#endif

void registerModules() 
{
  Appl.addConfigHandler("wifi",&oWiFiController);
  Appl.addConfigHandler("mqtt",&oMqttController);
  Appl.addConfigHandler("sensor",&oDHTSensor);


  Appl.addStatusHandler("wifi",&oWiFiController);
  Appl.addStatusHandler("mqtt",&oMqttController);
  Appl.addStatusHandler("sensor",&oDHTSensor);

  // Connect the message bus to the application control implementation
  Appl.MsgBus.registerEventReceiver(&oAppControl);
  Appl.MsgBus.registerEventReceiver(&oWiFiController);
  Appl.MsgBus.registerEventReceiver(&oMqttController);
  Appl.MsgBus.registerEventReceiver(&oWebSocket);   
  Appl.MsgBus.registerEventReceiver(&oStatusLED);
}

void setup() {
  Serial.begin(115200);
  DEBUG_FUNC_START();
  DEBUG_INFOS("\nInitializing application: \"%s\" Version: %s\n",APP_NAME,APP_VERSION);
  
  registerModules();

  oStatusLED.setColor(RGB_COLOR::BLUE);
  Appl.init(APP_NAME,APP_VERSION);
  Appl.sayHello();
  
  // If button is pressed during startup, do not load config (to allow recovery)
  bool bUseConfigData = !oBtn.isPressed();
  if(bUseConfigData) Appl.readConfigFrom(CONFIG_FILE);

  Appl.printDiag();
  Appl.Log.logInfo(F("Initializing services..."));
 
  // Initialize and start the WiFi
  oWiFiController.startWiFi(bUseConfigData);
  oMqttController.setup();
  
  // Prepare and start the Web Server
  // enable registerWebRoutes if you have custom web routes (WebRoutes.cpp/h)
  oWebServer.addHandler(&oWebSocket);
  registerWebRoutes(oWebServer);  
  oWebServer.begin();
  

  Appl.Log.logInfo(F("Hello world... - let's start the show!"));
  oStatusLED.showStartupFlashLight(250);
  delay(200);
  oBtn.startMonitoring();

  #ifdef DEBUGINFOS
    runDebugTests();
  #endif
  DEBUG_FUNC_END();
}

void loop() {
  // put your main code here, to run repeatedly:
  Appl.dispatch();
}
