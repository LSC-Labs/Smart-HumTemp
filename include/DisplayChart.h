#pragma once


#include <Appl.h>
#include <Display.h>
#include <SimpleDelay.h>
#include <DataStore.h>
#define INVALID_TEMPERATURE_DATA  -9999


class CDisplayChart : public IMsgEventReceiver {
    protected:
        CDataStore<int> m_oFinalTemperatureData;
        CDataStore<int> m_oPeriodTemperatureData;
/*
        int *m_pnTemperatureDataPoints = nullptr;
        int m_nActTemperatureDataPoint = -1;
        int m_nMaxTemperatureDataPoints = 0; 
*/
        int m_nSensorLocationID = -1;
        
        unsigned int m_nMeasurementTimeRange = 24 * 60 * 60 * 1000; // 24 hours in milliseconds
//        unsigned int m_nPeriodTimeSpan = 0;
        CSimpleDelay m_oPeriodTimeSpan;
//        int *m_pnPeriodTempDataPoints = nullptr;
//        unsigned int m_nActPeriodTempDataPoint = 0;
//        unsigned int m_nMaxPeriodTempDataPoints = 0;
 //        unsigned int m_nPeriodUpdateInterval = 1000;

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
            /*
            if(m_pnTemperatureDataPoints) {
                delete[] m_pnTemperatureDataPoints;
                m_pnTemperatureDataPoints = nullptr;
                m_nActTemperatureDataPoint = -1;
                m_nMaxTemperatureDataPoints = 0;
            }
                */
        };

        bool hasTemperatureDataPoints() {
            return(m_oFinalTemperatureData.count() > 0);
        }
/*
        void addNextTemperatureDataPoint() {
            int nNumOfPoints = 0;
            unsigned int nTotalTemp=0;
            if(m_pnTemperatureDataPoints && m_pnPeriodTempDataPoints) {
                for(unsigned int i=0; i < m_nActPeriodTempDataPoint; i++) {
                    nTotalTemp += m_pnPeriodTempDataPoints[i];
                    nNumOfPoints++;
                }
                int nAvgTemp = INVALID_TEMPERATURE_DATA;
                if(nNumOfPoints > 0) nAvgTemp = nTotalTemp / nNumOfPoints;
                // Check overflow
                m_nActTemperatureDataPoint++;
                if(m_nActTemperatureDataPoint >= m_nMaxTemperatureDataPoints) {
                    m_nActTemperatureDataPoint = 0;
                } 
                m_pnTemperatureDataPoints[m_nActTemperatureDataPoint] = nAvgTemp;
                // Reset period data points
                m_nActPeriodTempDataPoint = 0;
            }
        }
*/
        virtual int receiveEvent(const void * pSender, int nMsg, const void * pMessage, int nClass) override;
        virtual void onPeriodFinished();
        void dispatch();
    };
