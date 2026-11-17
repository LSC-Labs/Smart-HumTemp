#pragma once
/**
 * Board specific pin assignment.
 *
 * The LOLIN C3 Mini has the same form factor (header positions) as the Wemos D1 mini,
 * so the same wiring / shields can be used. The pins below are the GPIOs at the
 * same header positions as on the D1 mini.
 *
 *  Function          D1 mini (ESP8266)    LOLIN C3 Mini (ESP32-C3)
 *  ----------------  -------------------  ------------------------
 *  DHT data          D4 (GPIO2)           GPIO6
 *  Status LED red    D6 (GPIO12)          GPIO3
 *  Status LED green  D5 (GPIO14)          GPIO2
 *  Status LED blue   D0 (GPIO16)          GPIO1
 *  Display SDA       D2 (GPIO4)           GPIO8   (Wire default)
 *  Display SCL       D1 (GPIO5)           GPIO10  (Wire default)
 *
 * Each value can be overridden with a build flag, i.e.  -D PIN_DHT_SENSOR=7
 */
#include <Arduino.h>

#if defined(ARDUINO_LOLIN_C3_MINI)
    #ifndef PIN_DHT_SENSOR
        #define PIN_DHT_SENSOR  6
    #endif
    #ifndef PIN_LED_RED
        #define PIN_LED_RED     3
    #endif
    #ifndef PIN_LED_GREEN
        #define PIN_LED_GREEN   2
    #endif
    #ifndef PIN_LED_BLUE
        #define PIN_LED_BLUE    1
    #endif
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ESP8266)
    #ifndef PIN_DHT_SENSOR
        #define PIN_DHT_SENSOR  D4
    #endif
    #ifndef PIN_LED_RED
        #define PIN_LED_RED     D6
    #endif
    #ifndef PIN_LED_GREEN
        #define PIN_LED_GREEN   D5
    #endif
    #ifndef PIN_LED_BLUE
        #define PIN_LED_BLUE    D0
    #endif
#else
    #error "Unknown board - define the pins in BoardConfig.h or via build flags"
#endif
