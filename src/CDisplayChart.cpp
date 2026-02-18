#include <DisplayChart.h>
#include <DHTSensor.h>


int CDisplayChart::receiveEvent(const void * pSender, int nMsg, const void * pMessage, int nClass) {
    switch(nMsg) {
        case MSG_APPL_LOOP: dispatch(); break;
        case MSG_SENSOR_STATUS: {
            if(pMessage && nClass > 0) {
                DHTStatus *pStatus = (DHTStatus *) pMessage;
                if(pStatus->SensorLocation == m_nSensorLocationID) {
                    // Round the number up or down and use the integer part
                    // and insert it into the running period...
                    m_oPeriodTemperatureData.add((int) roundf(pStatus->TempC));   
                }
            }
        }
    }
    return(EVENT_MSG_RESULT_OK);
}

/**
 * Dispatch function
 * will be called by the framework (Appl.dispatch()) or
 * called by user on a regular base
 */
void CDisplayChart::dispatch() {
    if(m_oPeriodTimeSpan.isDone()) {
        onPeriodFinished();
        m_oPeriodTimeSpan.restart();
    }
}
 
/**
 * The period is finished...
 * Build the average about the temperature and move the
 * value into the display array(s)
 */
void CDisplayChart::onPeriodFinished() {
    int nAvgTemp = INVALID_TEMPERATURE_DATA;
    unsigned int nNumOfPoints = m_oPeriodTemperatureData.count();
    if(nNumOfPoints > 0) {
        int nTotalTemp = 0;
        for(unsigned int i=0; i < nNumOfPoints; i++) {
            nTotalTemp +=  m_oPeriodTemperatureData.getAt(i);
        }
        nAvgTemp = nTotalTemp / nNumOfPoints;
        m_oPeriodTemperatureData.clear();
    }
    m_oFinalTemperatureData.add(nAvgTemp);
}