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

class CDisplayBasePage : public CDisplayPage, public IMsgEventReceiver {
    protected:
        DisplayStatus Status;
        CSimpleDelay m_oDisplayRefresh;
        int m_nRSSI = 0;

    public:
        CDisplayBasePage(const char * pszName, PageType eType) : CDisplayPage(pszName,eType){init(pszName);};
        CDisplayBasePage(String strName, PageType eType) : CDisplayPage(strName,eType){init(strName.c_str());};
        int receiveEvent(const void * pSender, int nMsgType, const void * pMessage, int nClass) override;
        int drawPageHeader(CDisplay *pDisplay);
        int drawNumberFrame(CDisplay *pDisplay);

        enum Format {
            Temperature,
            Humidity
        };

        /**
         * Print the number as temp or as humidity into the buffer
         * - temp: check if +/- sign is needed
         */
        void sprintfDisplayNumber(char *pszBuffer, float fData, Format eFormat) {
            switch(eFormat) {
                case Temperature: // Display format as temperature (--.-)
                    if(isnan(fData)) sprintf(pszBuffer," --.-");
                    else sprintf(pszBuffer,"%2.1f",fData);
                    break;
                case Humidity:
                    if(isnan(fData)) sprintf(pszBuffer,"--");
                    else sprintf(pszBuffer,"%2.f",fData);
            }
        }

        void sprintfSensorDewPoint(char *pszBuffer,DHTStatus *pStatus) {
            if(pszBuffer) {
                float fData = pStatus ? pStatus->DewPoint ? pStatus->DewPoint : NAN : NAN;
                if(isnan(fData)) sprintf(pszBuffer,"(--.-)");
                else sprintf(pszBuffer,"(%2.1f)",fData);
            }
        }
         
        void sprintfSensorTemp(char *pszBuffer,DHTStatus *pStatus) {
            if(pszBuffer) {
                float fData = pStatus ? pStatus->TempC ? pStatus->TempC : NAN : NAN;
                if(isnan(fData)) sprintf(pszBuffer," --.-%cC",0xf8);
                else {
                    if(fData > -1) sprintf(pszBuffer," %2.1f%cC",fData,0xf8);
                    else sprintf(pszBuffer,"%2.1f%cC",fData,0xf8);
                }
            }
        }

        void sprintfSensorHumidity(char *pszBuffer,DHTStatus *pStatus) {
            if(pszBuffer) {
                float fData = pStatus ? pStatus->Humidity ? pStatus->Humidity : NAN : NAN;
                if(isnan(fData)) strcpy(pszBuffer," --%");
                else {
                    if(fData > -1) sprintf(pszBuffer,"%2.f%%",fData);
                    else sprintf(pszBuffer,"%2.f%%",fData);
                }
            }
        }

        void printTemperature(CDisplay *pDisplay, float fTemp) {
            if(isnan(fTemp)) pDisplay->printf(" --.- %cC",0xf8);
            else {
                if(fTemp > -1) pDisplay->printf(" %2.1f %cC",fTemp,0xf8);
                else pDisplay->printf("%2.1f %cC",fTemp,0xf8);
            }
        }

        void printSensorTemperature(CDisplay *pDisplay,DHTStatus *pStatus) {
            float fTemp = pStatus ? pStatus->TempC ? pStatus->TempC : NAN : NAN;
            printTemperature(pDisplay,fTemp);
        }

        void printSensorDewPoint(CDisplay *pDisplay,DHTStatus *pStatus) {
            float fTemp = pStatus ? pStatus->DewPoint ? pStatus->DewPoint : NAN : NAN;
            if(isnan(fTemp)) pDisplay->printf("--.- %c",0xf8);
            else pDisplay->printf("%2.1f %c",fTemp,0xf8);
        }

        void printSensorHumidity(CDisplay *pDisplay,DHTStatus *pStatus) {
            if(pDisplay) {
                float fValue = pStatus ? pStatus->Humidity ? pStatus->Humidity : NAN : NAN;
                if(isnan(fValue)) pDisplay->print("-- %");
                else pDisplay->printf("%2.f %%",fValue);
            }
        }
    protected:
        void init(const char * pszName);
        
};
