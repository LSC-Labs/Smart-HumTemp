#pragma once

#include <Arduino.h>
#include <Appl.h>
#include <AppConfig.h>
#include <ModuleInterface.h>
#include <MQTTController.h>
#include <JsonNode.h>
#include <SimpleDelay.h>

#include <DHT.h>


#ifndef LSC_DHT_DEFAULT_SENSOR
    #define LSC_DHT_DEFAULT_SENSOR DHT21
#endif

struct DHTStatus {
    const char * SENSOR_STATUS_TEMPC         = "temp_c";
    const char * SENSOR_STATUS_TEMPF         = "temp_f";
    const char * SENSOR_STATUS_HUMIDITY      = "hum";
    const char * SENSOR_STATUS_TEMPC_RAW     = "temp_c_raw";
    const char * SENSOR_STATUS_TEMPF_RAW     = "temp_f_raw";
    const char * SENSOR_STATUS_HUMIDITY_RAW  = "hum_raw";
    const char * SENSOR_STATUS_DEWPOINT_C    = "dew_c";
    const char * SENSOR_STATUS_ABS_HUMIDITY  = "abs_hum";
    const char * SENSOR_STATUS_LAST_UPD_MS   = "last_upd_ms";
    const char * SENSOR_STATUS_MONITORING    = "active";
    const char * SENSOR_STATUS_LOCATION      = "loc";

    bool  MonitoringActive = false;
    float TempRawC      = NAN;
    float TempRawF      = NAN;
    float HumidityRaw   = NAN;
    float TempC         = NAN;
    float TempF         = NAN;
    float Humidity      = NAN;
    float DewPoint      = NAN;
    float AbsHumidity   = NAN;      // absolute humidity in g/m³
    int           SensorLocation = -1;
    unsigned long LastWeatherCallMillis = 0;
    unsigned long LastSensorReadMillis  = 0;

    /**
     * ! will not write values that are set to NAN (JsonNode);
     */
    void writeStatusTo(CJsonNode & oStatus, int nLevel) {
        oStatus.setValue(SENSOR_STATUS_MONITORING,  MonitoringActive);
        oStatus.setValue(SENSOR_STATUS_LOCATION,    SensorLocation);
        oStatus.setValue(SENSOR_STATUS_TEMPC,       TempC);
        oStatus.setValue(SENSOR_STATUS_TEMPF,       TempF);
        oStatus.setValue(SENSOR_STATUS_HUMIDITY,    Humidity);
        oStatus.setValue(SENSOR_STATUS_DEWPOINT_C,    DewPoint);
        oStatus.setValue(SENSOR_STATUS_ABS_HUMIDITY,  AbsHumidity);
        if(nLevel >= STATUS_LEVEL_VERBOSE) {
            oStatus.setValue(SENSOR_STATUS_TEMPC_RAW, TempRawC);
            oStatus.setValue(SENSOR_STATUS_TEMPF_RAW, TempRawF);
            oStatus.setValue(SENSOR_STATUS_HUMIDITY_RAW, HumidityRaw);
            oStatus.setValue(SENSOR_STATUS_LAST_UPD_MS, LastWeatherCallMillis);
        }
    }

};

struct DHTConfig {
    bool    bIsPhysicalSensor   = true;
    int     sensorType          = LSC_DHT_DEFAULT_SENSOR;      // Sensor type to be used
    int     sensorPin           = -1;         // HW Pin
    float   adjustTempC         = 0.0;        // Temperature adjustment in °C
    float   adjustHumidity      = 0.0;        // Humidity adjustment
    /** only needed if open weather map is used **/
    // String strWeatherURL = "https://api.openweathermap.org/data/3.0/onecall";
    String strWeatherURL = "http://api.openweathermap.org/data/2.5/weather";
    // NOTE: Replace with your actual API key securely in production (e.g., via configuration file or environment variable)
    String strWeatherAppID = "<YOUR_OPENWEATHERMAP_API_KEY>";
    String strWeatherLongitude = "31.553641";
    String strWeatherLatitude  = "33.556778";
    String strWeatherExclude;   

    // Home Assistant Dewpoint Name in GUI - can be modified in Admin GUI
    String strHADewPointName = "Dewpoint";

    unsigned long nWeatherCallTimeout = 1000 * 60 * 2; // Call every 2 minutes to avoid the 1000 calls per day limit.
#ifdef DEBUGINFOS
    unsigned long nSensorReadTimeout = 20000; // Read the sensor in Debug Mode to avoid message flooding   
#else
    unsigned long nSensorReadTimeout = 10000; // Read the sensor every 10 seconds (if physical sensor is used)    
#endif
};

class CDHTSensor : public IModule, public IHomeAssistantComponent { //  : IConfigHandler, IStatusHandler {

    protected:
        DHTConfig    Config;
        CSimpleDelay m_oSensorReadDelay;      
        CSimpleDelay m_oUpdateDelay; // Update delay overall...
        DHT         *pSensor = nullptr; 
        bool         m_bInternetAvailable = false;
        JsonNode     OpenWeatherData;

    public:
        DHTStatus Status;

    public: 
        CDHTSensor();
        ~CDHTSensor();

        CDHTSensor(int nPort, int nLocation, int nSensorType = -1);
        void setup(int nPort, int nLocation, int nSensorType = -1);
        void startMonitoring(int nSensorType = -1);
        void dispatch();

        bool isMonitoringActive();

        // Interface implementations
        void writeStatusTo(CJsonNode & oObj, int nLevel = STATUS_LEVEL_INFO);
        void readConfigFrom(JsonNode & oCfg) override;
        void writeConfigTo(JsonNode  & oCfg, bool bHideCritical) override;
        int  receiveEvent(const void * pSender, int nMsg, const void * pData, int nType) override;

        float getTemperature(bool bAsFarenheit = false);
        float getHumidity();
        float getDewPoint(bool bInFarenheit = false);
        float calculateDewPoint(float fTemp, float fHum, bool bInFarenheit = false);
        float calculateAbsoluteHumidity(float fTempC, float fHum);
        bool  isInternetAvailable();
        // Home Assistant Component Interface implementation (if used)
//        void insertComponentDiscovery(const char * pszComponentName, JsonObject & oComponentArea) override;
        void insertComponentDiscovery(const char * pszComponentName,JsonNode & oComponents, CMQTTController *pController);
    protected:
        float adjustAndStoreTemperatures(float fRawTemp, bool bInFarenheit = false);
        float adjustAndStoreHumidity(float fRawHumi);
        void  updateFromPhysicalSensor();
        void  updateFromOpenWeatherMap();
};

