#pragma once

#include <DisplayPage.h>
#include <EventHandler.h>
#include <DHTSensor.h>


#define TEMPERATURE_SIGN  0xf8

struct DisplayStatus {
    DHTStatus *pSI              = nullptr;     // Status sensor indoor
    DHTStatus *pSO              = nullptr;     // Status sensor outdoor
    bool isRebootPending        = false;
    bool isButtonPressed        = false;
    bool isInAccessPointMode    = false;
    bool isInStationMode        = false;
    bool isWiFiConnected        = false;
    bool isFanActive            = false;
};

// Size of the text buffers for one formatted value (i.e. " -12.3\xf8C" / "(--.-)")
#define DISPLAY_VALUE_BUFFER_SIZE  16

/**
 * Base class of the sensor pages.
 *
 * RAM: the status is shared by all pages (static) and collected by ONE receiver
 *      on the message bus (registered with the first page) - before, each page
 *      was registered as own receiver and kept an own copy of the status.
 *      All format strings are in flash (PSTR / sprintf_P).
 */
class CDisplayBasePage : public CDisplayPage {
    protected:
        static DisplayStatus Status;            // shared by all sensor pages
        static CSimpleDelay  m_oDisplayRefresh; // RSSI refresh timer (shared)
        static int           m_nRSSI;

    public:
        CDisplayBasePage(const char * pszName, PageType eType) : CDisplayPage(pszName,eType){init();};
        static int receiveStatusEvent(const void * pSender, int nMsgType, const void * pMessage, int nClass);
        int drawPageHeader(CDisplay *pDisplay);
        int drawNumberFrame(CDisplay *pDisplay);

        enum Format {
            Temperature,
            Humidity
        };

        /**
         * Print the number as temp or as humidity into the buffer (min. DISPLAY_VALUE_BUFFER_SIZE)
         * - temp: check if +/- sign is needed
         */
        static void sprintfDisplayNumber(char *pszBuffer, float fData, Format eFormat) {
            switch(eFormat) {
                case Temperature: // Display format as temperature (--.-)
                    if(isnan(fData)) strcpy_P(pszBuffer,PSTR(" --.-"));
                    else sprintf_P(pszBuffer,PSTR("%2.1f"),fData);
                    break;
                case Humidity:
                    if(isnan(fData)) strcpy_P(pszBuffer,PSTR("--"));
                    else sprintf_P(pszBuffer,PSTR("%2.f"),fData);
            }
        }

        static void sprintfSensorDewPoint(char *pszBuffer,DHTStatus *pStatus) {
            if(pszBuffer) {
                float fData = pStatus ? pStatus->DewPoint ? pStatus->DewPoint : NAN : NAN;
                if(isnan(fData)) strcpy_P(pszBuffer,PSTR("(--.-)"));
                else sprintf_P(pszBuffer,PSTR("(%2.1f)"),fData);
            }
        }
         
        static void sprintfSensorTemp(char *pszBuffer,DHTStatus *pStatus) {
            if(pszBuffer) {
                float fData = pStatus ? pStatus->TempC ? pStatus->TempC : NAN : NAN;
                if(isnan(fData)) sprintf_P(pszBuffer,PSTR(" --.-%cC"),TEMPERATURE_SIGN);
                else {
                    if(fData > -1) sprintf_P(pszBuffer,PSTR(" %2.1f%cC"),fData,TEMPERATURE_SIGN);
                    else sprintf_P(pszBuffer,PSTR("%2.1f%cC"),fData,TEMPERATURE_SIGN);
                }
            }
        }

        static void sprintfSensorHumidity(char *pszBuffer,DHTStatus *pStatus) {
            if(pszBuffer) {
                float fData = pStatus ? pStatus->Humidity ? pStatus->Humidity : NAN : NAN;
                if(isnan(fData)) strcpy_P(pszBuffer,PSTR(" --%"));
                else sprintf_P(pszBuffer,PSTR("%2.f%%"),fData);
            }
        }

        static void printTemperature(CDisplay *pDisplay, float fTemp) {
            char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
            if(isnan(fTemp)) sprintf_P(szBuffer,PSTR(" --.- %cC"),TEMPERATURE_SIGN);
            else {
                if(fTemp > -1) sprintf_P(szBuffer,PSTR(" %2.1f %cC"),fTemp,TEMPERATURE_SIGN);
                else sprintf_P(szBuffer,PSTR("%2.1f %cC"),fTemp,TEMPERATURE_SIGN);
            }
            pDisplay->print(szBuffer);
        }

        static void printSensorTemperature(CDisplay *pDisplay,DHTStatus *pStatus) {
            float fTemp = pStatus ? pStatus->TempC ? pStatus->TempC : NAN : NAN;
            printTemperature(pDisplay,fTemp);
        }

        static void printSensorDewPoint(CDisplay *pDisplay,DHTStatus *pStatus) {
            char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
            float fTemp = pStatus ? pStatus->DewPoint ? pStatus->DewPoint : NAN : NAN;
            if(isnan(fTemp)) sprintf_P(szBuffer,PSTR("--.- %c"),TEMPERATURE_SIGN);
            else sprintf_P(szBuffer,PSTR("%2.1f %c"),fTemp,TEMPERATURE_SIGN);
            pDisplay->print(szBuffer);
        }

        static void printSensorHumidity(CDisplay *pDisplay,DHTStatus *pStatus) {
            if(pDisplay) {
                char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
                float fValue = pStatus ? pStatus->Humidity ? pStatus->Humidity : NAN : NAN;
                if(isnan(fValue)) strcpy_P(szBuffer,PSTR("-- %"));
                else sprintf_P(szBuffer,PSTR("%2.f %%"),fValue);
                pDisplay->print(szBuffer);
            }
        }
    protected:
        static void init();
        
};
