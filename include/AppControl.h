#pragma once

/**
 * Use the App to implement the main application logic.
 * Registered on the message bus, you can reflect to messages sent fromt the modules.
 */

#include <EventHandler.h>
#include <AppConfig.h>
#include <SimpleDelay.h>
#include <DHTSensor.h>
#include <queue>



class CAppControl : public IMsgEventReceiver
{
    protected:
        bool m_bRebootPending = false;      // Reboot is pending
        bool m_bButtonPressed = false;      // Button is currently pressed
        CSimpleDelay m_oButtonRestartDelay; // Delay to wait until button pressed is valid for restart
        DHTStatus *m_pSensorStatus = nullptr;
        std::queue<JsonDocument *> m_tStatusQueue;

    public:
        CAppControl();

        // Inherited via EventHandler
        virtual int receiveEvent(const void* pSender, int nMsgID, const void * pMsg, int nMsgClass);
        void dispatch();

    protected:
        void storeSensorStatus(DHTStatus *pStatus, int nSensor);
};
