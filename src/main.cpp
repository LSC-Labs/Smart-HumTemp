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

#include <AppConfig.h>
#include <Appl.h>
#include <AppControl.h>
#include <FileSystem.h>
#include <WiFiController.h>
#include <NTPHandler.h>

// #include <MDNSController.h>
// #include <CaptivePortal.h>
#include <MQTTController.h>
#include <WebServer.h>
#include <WebRoutes.h>
#include <DHTSensor.h>
#include <AppStatusLed.h>
// #include <Button.h>
#include <Display.h>
#include <DisplayPages.h>


// Instanziate the Sensor (Location 1, DHT22 on D4)
CDHTSensor oDHTSensor(D4,SENSOR_INDOOR); // GPIO2 , DHT22 Sensor, Location 1 
CFS oFS;
CWiFiController oWiFiController;
// CMDNSController oMDNSController; // Auto registers on Message Bus
// CCaptivePortal  oCaptivePortal;  // Auto registers on Message Bus
CMQTTController oMqttController;
CWebSocket      oWebSocket("/ws");
CWebServer      oWebServer(80);
CAppControl     oAppControl;
CAppStatusLed   oStatusLED(D6,D5,D0,false);
// CButton         oBtn(D7,true);
CDisplay        oDisplay;
CNTPHandler     oNTP;
// D8 - Ein / Aus Sensor Relais

#ifdef DEBUGINFOS
void runDebugTests() {
  DEBUG_INFO("Registered Event Receiver:");
  Appl.MsgBus.dumpReceiver();

  DEBUG_INFO("Registered Config Handler:");
  Appl.dumpConfigHandler();  

  JsonNode oCfgNode;
  Appl.writeConfigTo(oCfgNode,false);
  DEBUG_INFO("Current configuration:");
  Serial.println(oCfgNode.getAsJsonTextPretty());
  CJsonNode * pWiFiNode = oCfgNode.getObject("wifi");
  if(pWiFiNode) {
    Serial.println(pWiFiNode->getAsJsonText());
  }
}
#endif

void registerDisplayPages() {
  DEBUG_FUNC_START();

  oDisplay.registerPage(new CDisplayLogoPage(DISPLAY_START_PAGE));
  oDisplay.registerPage(new CDisplayAccessPointPage("APConfig-Page"));
  // oDisplay.registerPage(new CDisplayDewPointPage("DewPointPage"));
  // oDisplay.registerPage(new CDisplayTempPage("TemperaturePage"));
  oDisplay.registerPage(new CDisplayTempPageLarge("LargeTempPage"));
  oDisplay.registerPage(new CDisplayHumidityPageLarge("LargeHumPage"));
  
  DEBUG_FUNC_END();
}


void registerModules() 
{
  DEBUG_FUNC_START();
  Appl.registerModule("wifi",          &oWiFiController);
  Appl.registerModule("web",           &oWebServer);
  Appl.registerModule("mqtt",          &oMqttController);
  // Appl.registerModule("captiveportal", &oCaptivePortal);
  Appl.registerModule("sensor",        &oDHTSensor);
  Appl.registerModule("ntp",           &oNTP);

  // Appl.addConfigHandler("display",    &oDisplay);

  // Connect the message bus to the application control implementation
  Appl.MsgBus.registerEventReceiver(&oAppControl,"AppCtrl");
  Appl.MsgBus.registerEventReceiver(&oStatusLED,"StatusLED");
  Appl.MsgBus.registerEventReceiver(&oWebSocket,"WebSocket");

  // Register the wifi to the mqtt Home Assistant auto discovery.
  oMqttController.registerHomeAssistantComponent("wifi",  &oWiFiController);
  oMqttController.registerHomeAssistantComponent("sensor",&oDHTSensor);
  DEBUG_FUNC_END();
}

void setup() {
  DEBUG_FUNC_START();
  DEBUG_INFOS("\nInitializing application: \"%s\" Version: %s\n",APP_NAME,APP_VERSION);

  // Initialize a simple feedback to the user...
  // set the status led to blue
  // and enable the display to show a startup panel...
  oStatusLED.setColor(RGB_COLOR::BLUE);
  registerDisplayPages();
  oDisplay.init();
  
  // Register all modules and simple receivers to the main application
  // - do this first, so they can receive the initialization messages.
  registerModules();

  // Initialize the main application object
  Appl.init(APP_NAME,APP_VERSION);
  Appl.sayHello();
  
  // If button is pressed during startup, do not load config (to allow recovery)
  // bool bUseConfigData = !oBtn.isPressed();
  bool bUseConfigData = true;
  if(bUseConfigData) Appl.readConfigFrom(CONFIG_FILE);

  
  // Show some diagnostic info on the serial bus 
  Appl.printDiag();
  ApplLogInfo(F("Initializing services..."));
 
  // Initialize and start the WiFi
  oWiFiController.startWiFi(bUseConfigData);

  // Setup the web server and the web socket...
  // enable registerWebRoutes if you have custom web routes (WebRoutes.cpp/h)
  registerWebRoutes(oWebServer);  
  oWebServer.addHandler(&oWebSocket);
  oWebServer.begin();

  if(oWiFiController.Status.isInAccessPointMode) {
    delay(500);
    // oDisplay.activatePage("APConfig-Page");
    // oDisplay.freezePage();
  } else {
    // Message broker only, if in station mode...
    oMqttController.setup();
  }

  // Be friendly, say hello and let start....
  Appl.Log.logInfo(F("Hello world... - let's start the show!"));
  oStatusLED.showStartupFlashLight(250);
  delay(200);

  // if there is a button, start monitoring (interrupt) now
  // oBtn.startMonitoring();
  oDHTSensor.startMonitoring();

  #ifdef DEBUGINFOS
    runDebugTests();
  #endif

  Appl.start();
  
  DEBUG_FUNC_END();
}

void loop() {
  // put your main code here, to run repeatedly
  Appl.dispatch();
}
