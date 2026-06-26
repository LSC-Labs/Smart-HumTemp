/**
 * CAppControl.cpp
 * 
 * Application Control Module
 * Use this module to implement the main application logic.
 */

#include <AppControl.h>
#include <Appl.h>



CAppControl::CAppControl()
{
    // Constructor implementation (if needed)
    m_oButtonRestartDelay.start(5000,false);
}   


int CAppControl::receiveEvent(const void* pSender, int nMsgID, const void * pMsg, int nMsgClass)
{
    int nResult = EVENT_MSG_RESULT_OK;
    // Handle incoming events here
    // Example:
    // if (nMsgID == SOME_MESSAGE_ID) { ... }
    switch(nMsgID)
    {
        case MSG_APPL_INITIALIZED:
            // Process when the application is up and running
            break;
        case MSG_RESTART_REQUEST:
            // Process some save actions if needed and prepare shutdown
            break;
        case MSG_BUTTON_ON:
            // Button pressed - start delay for restart
            m_bButtonPressed = true;
            m_oButtonRestartDelay.restart(); 
            break;
        case MSG_SENSOR_STATUS:
            // Status of a sensor received...
            storeSensorStatus((DHTStatus *) pMsg, nMsgClass);
            break;

        case MSG_APPL_LOOP:
            // Main loop processing
            dispatch();
            break;

    }
    return nResult; // Return appropriate status
}

void CAppControl::storeSensorStatus(DHTStatus * pStatus, int nSensorID) {
    if(pStatus) {
        CJsonNode oStatus;
        oStatus.setValue("id",nSensorID);
        pStatus->writeStatusTo(oStatus,STATUS_LEVEL_INFO);
        m_tStatusQueue.push(strdup(oStatus.getAsJsonText()));
    }
}

void CAppControl::dispatch()
{
    // Implement periodic tasks or message dispatching here
    // This function can be called regularly in the main loop
    // When button is pressed long enough - restart...
    if(m_bButtonPressed) {
        if(m_oButtonRestartDelay.isDone()) {
            m_bRebootPending = true;
            Appl.MsgBus.sendEvent(this,MSG_REBOOT_REQUEST,0,0);
        }
    } 

    // Dispatch the status documents... if in the queue...
    while(!m_tStatusQueue.empty()) {
		const char * pszStatus = m_tStatusQueue.front();
        Appl.MsgBus.sendEvent(this,MSG_MQTT_SEND_TEXTSTATE,pszStatus,MSG_JSON_OBJECT);
        free((void *)pszStatus);
        // delete(pMessageToProcess);
		m_tStatusQueue.pop();
	}


}