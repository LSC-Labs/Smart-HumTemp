#ifndef DEBUG_LSC_DHT_SENSOR
    #undef DEBUGINFOS
#endif

#include <LSCUtils.h>
#include <DHTSensor.h>
#include <Network.h>
#include <ESP8266HTTPClient.h>
#include <Msgs.h>
#include <JsonNode.h>

const char * SENSOR_CFG_IS_SENSOR        = "physical";
const char * SENSOR_CFG_SENSOR_TYPE      = "type";
const char * SENSOR_CFG_ADJUST_TEMP      = "adjust_temp";
const char * SENSOR_CFG_ADJUST_HUMIDITY  = "adjust_humidity";
const char * SENSOR_CFG_OPENWEATHER_URL  = "web_url";
const char * SENSOR_CFG_OPENWEATHER_KEY  = "web_key";
const char * SENSOR_CFG_OPENWEATHER_LON  = "web_lon";
const char * SENSOR_CFG_OPENWEATHER_LAT  = "web_lat";
const char * SENSOR_STATUS_INTERNET      = "internet";

CDHTSensor::CDHTSensor() {};
CDHTSensor::~CDHTSensor() { if(pSensor != nullptr) free(pSensor); }

CDHTSensor::CDHTSensor(int nPin, int nLocation, int nSensorType) {
    DEBUG_FUNC_START();
    m_oUpdateDelay.start(5000);
    setup(nPin,nLocation,nSensorType);
};

void CDHTSensor::setup(int nPin, int nLocation, int nSensorType) {
    DEBUG_FUNC_START_PARMS("%d,%d,%d",nPin,nLocation,nSensorType);
    if(nSensorType > -1)    Config.sensorType = nSensorType;
    Config.sensorPin        = nPin;
    Status.SensorLocation   = nLocation;  
    DEBUG_FUNC_END();
};

/**
 * @brief Begin monitoring
 */
void CDHTSensor::startMonitoring(int nSensorType) {
    DEBUG_FUNC_START_PARMS("%d",nSensorType);
    if(nSensorType > -1) Config.sensorType = nSensorType;
    if(Config.sensorType > -1) {
        if(Config.sensorType == USE_OPEN_WEATHER) {
            Config.bIsPhysicalSensor = false;
            m_oSensorReadDelay.start(this->Config.nWeatherCallTimeout);
        } else {
            if(Config.sensorPin > -1) {
                Config.bIsPhysicalSensor = true;
                if(pSensor) free(pSensor);  // Maybe new call with new sensor type...
                pSensor =  new DHT(Config.sensorPin,Config.sensorType);
                pSensor->begin();
                m_oSensorReadDelay.start(this->Config.nSensorReadTimeout);
            } else {
                DEBUG_INFOS("- Monitoring not started, missing information...")
                DEBUG_INFOS("  - Pin(%d) / Type(%d) must be set correctly (>0)",Config.sensorPin,Config.sensorType);

            }
        }
        Status.MonitoringActive = true;
    } 
    DEBUG_FUNC_END();
}

bool CDHTSensor::isMonitoringActive() { return(Status.MonitoringActive); }


void CDHTSensor::readConfigFrom(JsonNode & oCfg) {
    DEBUG_FUNC_START();
    DEBUG_INFOS("--- sensor adjust temp value : %f",Config.adjustTempC);
    DEBUG_JSON_OBJ(oCfg);
    
    oCfg.storeValueIf(SENSOR_CFG_IS_SENSOR,        &   Config.bIsPhysicalSensor);
    oCfg.storeValueIf(SENSOR_CFG_SENSOR_TYPE,      &   Config.sensorType);
    oCfg.storeValueIf(SENSOR_CFG_ADJUST_HUMIDITY,  &   Config.adjustHumidity);
    oCfg.storeValueIf(SENSOR_CFG_ADJUST_TEMP,      &   Config.adjustTempC);
    oCfg.storeValueIf(SENSOR_CFG_OPENWEATHER_KEY,      Config.strWeatherAppID);
    oCfg.storeValueIf(SENSOR_CFG_OPENWEATHER_LON,      Config.strWeatherLongitude);
    oCfg.storeValueIf(SENSOR_CFG_OPENWEATHER_LAT,      Config.strWeatherLatitude);

    m_oSensorReadDelay.start( Config.bIsPhysicalSensor      ? 
                            Config.nSensorReadTimeout   :
                            Config.nWeatherCallTimeout  )->setExpired();
    #ifdef DEBUGINFOS
        DEBUG_INFOS("--- sensor adjust temp value : %f",Config.adjustTempC);
        m_oSensorReadDelay.printDiag();
    #endif
    DEBUG_FUNC_END();
}

void CDHTSensor::writeConfigTo(JsonNode & oCfg, bool bHideCritical) {
    DEBUG_FUNC_START();
    oCfg[SENSOR_CFG_IS_SENSOR]          = Config.bIsPhysicalSensor;
    oCfg[SENSOR_CFG_SENSOR_TYPE]        = Config.sensorType;
    oCfg[SENSOR_CFG_ADJUST_HUMIDITY]    = Config.adjustHumidity;
    oCfg[SENSOR_CFG_ADJUST_TEMP]        = Config.adjustTempC;
    oCfg[SENSOR_CFG_OPENWEATHER_KEY]    = Config.strWeatherAppID;
    oCfg[SENSOR_CFG_OPENWEATHER_LON]    = Config.strWeatherLongitude;
    oCfg[SENSOR_CFG_OPENWEATHER_LAT]    = Config.strWeatherLatitude;
    DEBUG_FUNC_END();
}


void CDHTSensor::writeStatusTo(CJsonNode & oStatus, int nLevel) {
    if(nLevel >= STATUS_LEVEL_INFO) {
        oStatus.setValue(SENSOR_CFG_IS_SENSOR,Config.bIsPhysicalSensor ? "1" : "0");
    }
    if(nLevel >= STATUS_LEVEL_VERBOSE) {
        if(!Config.bIsPhysicalSensor) {
            oStatus[SENSOR_STATUS_INTERNET]     = isInternetAvailable();
        }
    }
    Status.writeStatusTo(oStatus,nLevel);
}

/// @brief Listen to the messagebus - detect if internet is available or not.
/// @param pSender 
/// @param nMsg 
/// @param pData 
/// @param nType 
/// @return 
int CDHTSensor::receiveEvent(const void * pSender, int nMsg, const void * pData, int nType) {
    switch(nMsg) {
        case MSG_APPL_LOOP:         dispatch();
                                    break;

        case MSG_WIFI_CONNECTED:    m_bInternetAvailable = (nType == WIFI_STATION_MODE);
            DEBUG_INFOS(" ## -> Setting Internet access to %d",isInternetAvailable());
                                    break;

        case MSG_WIFI_DISABLED :    m_bInternetAvailable = false;
            DEBUG_INFO(" ## -> Disabling Internet access for sensors...");
                                    break;
    }
    return(EVENT_MSG_RESULT_OK);
}

// Messages for Home Assistance
// -> https://github.com/home-assistant/core/blob/dev/homeassistant/components/mqtt/abbreviations.py
// -> https://www.home-assistant.io/integrations/mqtt
//
// Abbreviated keys are used to keep the discovery document small (heap on ESP8266):
//   p = platform, dev_cla = device_class, stat_cla = state_class, unit_of_meas = unit_of_measurement,
//   val_tpl = value_template, uniq_id = unique_id, sug_dsp_prc = suggested_display_precision, ic = icon
//
// Values that are NAN are not written into the state (see DHTStatus::writeStatusTo()).
// The template therefore checks "is defined" and renders "None" => HA shows the entity as "unknown"
// instead of logging template errors (and storing wrong values in the statistics).
static void insertHASensorComponent(JsonNode & oComponents,
                                    const char * pszComponentName,
                                    const char * pszValueKey,
                                    const char * pszDeviceClass,
                                    const char * pszUnit,
                                    const char * pszName = nullptr,
                                    const char * pszIcon = nullptr) {
    char szValueTemplate[192];
    snprintf(szValueTemplate,sizeof(szValueTemplate),
             "{{ value_json.%s.%s | float | round(2) if value_json.%s.%s is defined else None }}",
             pszComponentName, pszValueKey, pszComponentName, pszValueKey);
    String strUniqueID = Appl.getDeviceID();
    strUniqueID += "-";
    strUniqueID += pszValueKey;

    JsonNode * pSensor = oComponents.getObject(pszValueKey,true);
    if(!pSensor) return;
    pSensor->setValue("p",              "sensor");
    pSensor->setValue("dev_cla",        pszDeviceClass);
    pSensor->setValue("stat_cla",       "measurement");     // required for long term statistics / history graphs
    pSensor->setValue("unit_of_meas",   pszUnit);
    pSensor->setValue("sug_dsp_prc",    1);
    pSensor->setValue("val_tpl",        szValueTemplate);
    pSensor->setValue("uniq_id",        strUniqueID.c_str());
    // Without a name HA uses the translated device class name ("Temperature", "Humidity")
    if(pszName && *pszName) pSensor->setValue("name", pszName);
    if(pszIcon && *pszIcon) pSensor->setValue("ic",   pszIcon);
}

void CDHTSensor::insertComponentDiscovery(const char * pszComponentName,JsonNode & oComponents, CMQTTController *pController) {
    (void) pController;
    if(!pszComponentName || !*pszComponentName) return;
    // Keys must match DHTStatus (SENSOR_STATUS_HUMIDITY / _TEMPC / _DEWPOINT_C)
    insertHASensorComponent(oComponents, pszComponentName, Status.SENSOR_STATUS_HUMIDITY,   "humidity",    "%");
    insertHASensorComponent(oComponents, pszComponentName, Status.SENSOR_STATUS_TEMPC,      "temperature", "°C");
    insertHASensorComponent(oComponents, pszComponentName, Status.SENSOR_STATUS_DEWPOINT_C, "temperature", "°C",
                            Config.strHADewPointName.c_str(), "mdi:water-thermometer-outline");
}


void CDHTSensor::dispatch() {
    // Currently no periodic tasks needed.
    getDewPoint();
}   

bool CDHTSensor::isInternetAvailable() {
    return(this->m_bInternetAvailable);
}

/// @brief Stores the temperature in the Status Object
/// @param fRawData 
/// @param bAsFarenheit if true, fRawData is in Fahrenheit
/// @return The adjusted temperature in Celsius or Fahrenheit
float CDHTSensor::adjustAndStoreTemperatures(float fRawData, bool bAsFarenheit) {
    DEBUG_FUNC_START_PARMS("fRawData=%f,bAsFarenheit=%d",fRawData,bAsFarenheit);
    if(!isnan(fRawData)) {
        if(bAsFarenheit) {
            Status.TempRawF =  fRawData;
            Status.TempF    = fRawData + LSC::getFarenheitFromCelsius(Config.adjustTempC);
            Status.TempRawC = LSC::getCelsiusFromFarenheit(fRawData);
            Status.TempC    = LSC::getCelsiusFromFarenheit(Status.TempRawF);
        } else {
            Status.TempRawC = fRawData;
            Status.TempC    = fRawData + Config.adjustTempC;
            Status.TempRawF = LSC::getFarenheitFromCelsius(fRawData);
            Status.TempF    = LSC::getFarenheitFromCelsius(Status.TempC);
        }
    } 
    DEBUG_FUNC_END_PARMS("%f",bAsFarenheit ? Status.TempF : Status.TempC);
    return(bAsFarenheit ? Status.TempF : Status.TempC);
}

/// @brief Stores the humidity in the Status Object.
/// @param fRawData 
/// @return The adjusted value - or if no value is given, the current humidity.
float CDHTSensor::adjustAndStoreHumidity(float fRawData) {
    DEBUG_FUNC_START_PARMS("fRawData=%f",fRawData);
    if(!isnan(fRawData)) {
        Status.HumidityRaw = fRawData;
        Status.Humidity = fRawData + Config.adjustHumidity;
    }
    DEBUG_FUNC_END_PARMS("%f",Status.Humidity);
    return(Status.Humidity);
}

void CDHTSensor::updateFromPhysicalSensor() {
    DEBUG_FUNC_START();
    if(this->pSensor) {
        if(m_oSensorReadDelay.isDone()) {
            DEBUG_INFO(" ->> reading physical sensor");
            // Force reading in Celsius
            try {
                float fTemp = pSensor->readTemperature(false);
                float fHumi = pSensor->readHumidity();
                DEBUG_INFOS("Sensor (phys): %f - %f",fTemp,fHumi);
                adjustAndStoreTemperatures(fTemp,false);
                adjustAndStoreHumidity(fHumi);
            } catch(...) {
                Serial.println(F("[E] Exception while reading physical sensor"));
            }
            Status.LastSensorReadMillis = m_oSensorReadDelay.restart();
        } else {
            DEBUG_INFOS(" -> Next sensor read in %lu ms", m_oSensorReadDelay.getRemaining());
        }
    }
    DEBUG_FUNC_END();
}   

// units=metric <- Default is kelvin !
// https://api.openweathermap.org/data/2.5/weather?lat=48.251690&lon=11.553641&appid=bf2414e748fff1ee23a45cd32bc61f90
// {"coord":{"lon":11.5536,"lat":48.2517},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":284.94,"feels_like":283.69,"temp_min":283.79,"temp_max":285.41,"pressure":1022,"humidity":58,"sea_level":1022,"grnd_level":963},"visibility":10000,"wind":{"speed":3.55,"deg":271,"gust":5.75},"clouds":{"all":1},"dt":1744358722,"sys":{"type":2,"id":2013241,"country":"DE","sunrise":1744345891,"sunset":1744394260},"timezone":7200,"id":2859147,"name":"Oberschleißheim
void  CDHTSensor::updateFromOpenWeatherMap() {
    DEBUG_FUNC_START();
   
    if(isInternetAvailable()) {
    
        if(m_oSensorReadDelay.isDone()) {
            try {
                DEBUG_INFOS(" ->> calling weather API (%lu + %lu) > %lu",Status.LastWeatherCallMillis,Config.nWeatherCallTimeout, millis());
                String strCallUrl = Config.strWeatherURL + 
                                    "?units=metric"
                                    "&lat=" + Config.strWeatherLatitude + 
                                    "&lon=" + Config.strWeatherLongitude +
                                    "&appid=" + Config.strWeatherAppID +
                                    "&exclude" + Config.strWeatherExclude;
                // TODO: Implement the call here...
                //       - send result via MQTT
                //       - store temperature and humidity in status...
                // Back from https - throws exceptions (!)
                DEBUG_INFOS("- Calling : %s", strCallUrl.c_str());
                // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
                // client->setInsecure();
                HTTPClient oClient;
                WiFiClient oWiFiClient;
                //oClient.begin(*client,strCallUrl);
                oClient.begin(oWiFiClient,strCallUrl);
                int nCode = oClient.GET();
                String strBody = oClient.getString();
                oClient.end();
                DEBUG_INFOS("Received code : %d", nCode);
                DEBUG_INFOS("Received data :\n%s",strBody.c_str());
                if(nCode == 200) {
                    OpenWeatherData.clear();
                    OpenWeatherData.parse(strBody.c_str());
                    // deserializeJson(OpenWeatherData,strBody);
                    // OpenWeatherData["c"] = APP_COPYRIGHT;
                    // OpenWeatherData.set(strBody.c_str());
                    Appl.MsgBus.sendEvent(this,MSG_OPEN_WEATHER_DATA,&OpenWeatherData,Status.DewPoint);
                    JsonNode * pMain = OpenWeatherData.getObject("main");
                    if(pMain) {
                        DEBUG_JSON_OBJ(*pMain);
                        float fTempC = pMain->getValueAsFloat("temp");
                        float fHumi  = pMain->getValueAsFloat("humidity");
                        adjustAndStoreTemperatures(fTempC,false);
                        adjustAndStoreHumidity(fHumi);
                    }
                }
            } catch(...) {
                Serial.println(F("[E] Exception in requesting data from Open weather"));
            }
            Status.LastWeatherCallMillis = m_oSensorReadDelay.restart();
        } else {
            DEBUG_INFOS(" -> Next internet call in %lu ms", m_oSensorReadDelay.getRemaining());
        }
        
    }
    DEBUG_FUNC_END();
}

/// @brief Get the current Humidity and stores the adjusted values into the Status object
/// @return the current (adjusted) humidity
float CDHTSensor::getHumidity() {
    DEBUG_FUNC_START();
    if(this->pSensor) {
        updateFromPhysicalSensor();
    }
    else {
        updateFromOpenWeatherMap();
    }
    DEBUG_FUNC_END_PARMS(" - humidity    : %f",Status.Humidity);
    return(Status.Humidity);
}

/// @brief Get the current Temperatur and stores the adjusted values into the Status object
/// @param bAsFarenheit Retur value in Celsius or Fahrenheit
/// @return the current (adjusted) temperature
float CDHTSensor::getTemperature(bool bAsFarenheit) {
    DEBUG_FUNC_START_PARMS("bAsFarenheit=%d",bAsFarenheit);
    if(isMonitoringActive()) {
        if(this->pSensor) {
            updateFromPhysicalSensor();
        }
        else {
            updateFromOpenWeatherMap();
        }
    }
    float fResult = bAsFarenheit ? Status.TempF : Status.TempC;
    DEBUG_FUNC_END_PARMS(" - temperature : %f",fResult);

    return(fResult);
}

/// @brief Dew Point temperature calculate
/// @param fTemp Temperature in celsius or farenheit
/// @param fHumidity Humidity
/// @param bAsFarenheit fTemp is in Farenheit ?
/// @return Dew Point Temperature in Celsius or Farenheit (depends on bAsFahrenheit)
float CDHTSensor::calculateDewPoint(float fTemp, float fHumidity, bool bAsFarenheit) {
    if(bAsFarenheit) fTemp = LSC::getCelsiusFromFarenheit(fTemp);
    DEBUG_FUNC_START_PARMS("%f,%f",fTemp,fHumidity);
    float a, b;
  
    if (fTemp >= 0) {
        a = 7.5;
        b = 237.3;
    } else {
        a = 7.6;
        b = 240.7;
    }
  
    // Sättigungsdampfdruck in hPa
    float sdd = 6.1078 * pow(10, (a*fTemp)/(b+fTemp));
    
    // Dampfdruck in hPa
    float dd = sdd * (fHumidity/100);
    
    // v-Parameter
    float v = log10(dd/6.1078);
    
    // Taupunkttemperatur (°C)
    float tt = (b*v) / (a-v);

    DEBUG_FUNC_END_PARMS("%f",bAsFarenheit ? LSC::getFarenheitFromCelsius(tt) : tt);
    return { bAsFarenheit ? LSC::getFarenheitFromCelsius(tt) : tt }; 
}

/// @brief Updates Temp and Humidity, calculates the Dew Point and sends the
///        SensorStatus object via the message bus.
float CDHTSensor::getDewPoint(bool bInFarenheit) {
    float fResult = Status.DewPoint;
    if(isMonitoringActive()) {
        if(m_oUpdateDelay.isDone()) {
            DEBUG_INFOS("SENSOR %d - start getDewPoint(%d)",Status.SensorLocation,bInFarenheit);
            getTemperature();
            getHumidity();
            if(!isnan(Status.TempC) && !isnan(Status.Humidity)) {
                Status.DewPoint = calculateDewPoint(Status.TempC,Status.Humidity,bInFarenheit);
                fResult = Status.DewPoint; 
            }
            Appl.MsgBus.sendEvent(this,MSG_SENSOR_STATUS,&Status,Status.SensorLocation);
            m_oUpdateDelay.restart();
            DEBUG_INFOS("SENSOR %d - end  getDewPoint()",Status.SensorLocation);
        } 
    }
    return(fResult);
}