#pragma once


#include <Appl.h>
#include <Display.h>
#include <SimpleDelay.h>
#include <DataStore.h>
#define INVALID_TEMPERATURE_DATA  -9999



class CDisplayChart : public IMsgEventReceiver {
    public:
    enum DataType {
        TEMP,
        HUMIDITY
    };
    protected:
        CDataStore<int> m_oFinalTemperatureData;
        CDataStore<int> m_oPeriodTemperatureData;

        int m_nSensorLocationID = -1;
        DataType m_eDataType = DataType::TEMP;
        
        unsigned int m_nMeasurementTimeRange = 24 * 60 * 60 * 1000; // 24 hours in milliseconds

        /**
         * Time span for a period to collect data, before storing the average
         * to the final data table...
         */
        CSimpleDelay m_oPeriodTimeSpan;

    public:
        CDisplayChart(int nSensorID,int nDataPoints = SCREEN_WIDTH,unsigned int nUpdateInterval = 1000) {
            // React only on the sensor with id == nSensorID
            // Allocate data points array with screen width size
            m_nSensorLocationID = nSensorID;
            m_oFinalTemperatureData.setSize(nDataPoints);

            // Calculate the time span per data point
            // If measurement interval is 24 hours and we have e.g. 128 data points, 
            // each point represents 24h / 128 = 11.25 minutes
            int nPeriodTimeSpan = m_nMeasurementTimeRange / nDataPoints;
            m_oPeriodTimeSpan.start(nPeriodTimeSpan,true);

            // Store data points for the update intervall
            // m_nMaxPeriodTempDataPoints = nUpdateInterval / 1000; // e.g. 1 second update interval -> 1000 data points
            // m_pnPeriodTempDataPoints = new int[m_nMaxPeriodTempDataPoints]; 
            
            int nUpdatePoints = nUpdateInterval / 100;
            m_oPeriodTemperatureData.setSize(nUpdatePoints);
        };

        ~CDisplayChart() {
           
        };

        bool hasTemperatureDataPoints() {
            return(m_oFinalTemperatureData.count() > 0);
        }

        void collectDataType(DataType eDataType){
            this->m_eDataType = eDataType;
            clearData();
        }
        virtual int receiveEvent(const void * pSender, int nMsg, const void * pMessage, int nClass) override;
        virtual void onPeriodFinished();
        void dispatch();
        void clearData() {
            this->m_oPeriodTemperatureData.clear();
            this->m_oFinalTemperatureData.clear();
        }
    };
