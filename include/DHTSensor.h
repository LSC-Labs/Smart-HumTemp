#pragma once

#include <Arduino.h>
#include <Appl.h>
#include <AppConfig.h>
#include <ModuleInterface.h>
#include <SimpleDelay.h>
#include <DHT.h>

struct DHTStatus {
    const char * SENSOR_STATUS_TEMPC         = "temp_c";
    const char * SENSOR_STATUS_TEMPF         = "temp_f";
    const char * SENSOR_STATUS_HUMIDITY      = "hum";
    const char * SENSOR_STATUS_TEMPC_RAW     = "temp_c_raw";
    const char * SENSOR_STATUS_TEMPF_RAW     = "temp_f_raw";
    const char * SENSOR_STATUS_HUMIDITY_RAW  = "hum_raw";
    const char * SENSOR_STATUS_DEWPOINT      = "dew_point";
    const char * SENSOR_STATUS_LAST_UPD_MS   = "last_upd_ms";

    float TempRawC      = NAN;
    float TempRawF      = NAN;
    float HumidityRaw   = NAN;
    float TempC         = NAN;
    float TempF         = NAN;
    float Humidity      = NAN;
    float DewPoint      = NAN;
    int           SensorLocation = -1;
    unsigned long LastWeatherCallMillis = 0;
    unsigned long LastSensorReadMillis  = 0;

    void writeStatusTo(JsonObject & oStatus) {
        oStatus[SENSOR_STATUS_TEMPC]        = TempC;
        oStatus[SENSOR_STATUS_TEMPC_RAW]    = TempRawC;
        oStatus[SENSOR_STATUS_TEMPF]        = TempF;
        oStatus[SENSOR_STATUS_TEMPF_RAW]    = TempRawF;
        oStatus[SENSOR_STATUS_HUMIDITY]     = Humidity;
        oStatus[SENSOR_STATUS_HUMIDITY_RAW] = HumidityRaw;
        oStatus[SENSOR_STATUS_DEWPOINT]     = DewPoint;
        oStatus[SENSOR_STATUS_LAST_UPD_MS]  = LastWeatherCallMillis;
    }

};

struct DHTConfig {
    bool bIsPhysicalSensor  = true;
    float adjustTempC       = 0.0;        // Temperature adjustment in °C
    float adjustHumidity    = 0.0;     // Humidity adjustment
    /** only needed if open weather map is used **/
    // String strWeatherURL = "https://api.openweathermap.org/data/3.0/onecall";
    String strWeatherURL = "http://api.openweathermap.org/data/2.5/weather";
    // NOTE: Replace with your actual API key securely in production (e.g., via configuration file or environment variable)
    String strWeatherAppID = "<YOUR_OPENWEATHERMAP_API_KEY>";
    String strWeatherLongitude = "31.553641";
    String strWeatherLatitude  = "33.556778";
    String strWeatherExclude;   
    unsigned long nWeatherCallTimeout = 1000 * 60 * 2; // Call every 2 minutes to avoid the 1000 calls per day limit.
#ifdef DEBUGINFOS
    unsigned long nSensorReadTimeout = 20000; // Read the sensor in Debug Mode to avoid message flooding   
#else
    unsigned long nSensorReadTimeout = 10000; // Read the sensor every 10 seconds (if physical sensor is used)    
#endif
};

class CDHTSensor : public IModule { //  : IConfigHandler, IStatusHandler {

    protected:
        DHTConfig Config;
        CSimpleDelay m_oSensorReadDelay;      
        CSimpleDelay m_oUpdateDelay; // Update delay overall...
        DHT *pSensor = nullptr; 
        bool m_bInternetAvailable = false;
        JsonDocument OpenWeatherData;

    public:
        DHTStatus Status;

    public: 
        CDHTSensor();
        ~CDHTSensor();
        CDHTSensor(int nPort, int nSensorType, int nLocation);

        void begin(int nPort, int nSensorType, int nLocation);
        void dispatch();

        // Interface implementations
        void writeStatusTo(JsonObject & oObj) override;
        void readConfigFrom(JsonObject & oCfg) override;
        void writeConfigTo(JsonObject & oCfg, bool bHideCritical) override;
        int  receiveEvent(const void * pSender, int nMsg, const void * pData, int nType) override;

        float getTemperature(bool bAsFarenheit = false);
        float getHumidity();
        float getDewPoint(bool bInFarenheit = false);
        float calculateDewPoint(float fTemp, float fHum, bool bInFarenheit = false);
        bool  isInternetAvailable();

    protected:
        float adjustAndStoreTemperatures(float fRawTemp, bool bInFarenheit = false);
        float adjustAndStoreHumidity(float fRawHumi);
        void  updateFromPhysicalSensor();
        void  updateFromOpenWeatherMap();
        
};

