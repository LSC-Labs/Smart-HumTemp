#pragma once

#include <SysStatusLed.h>

class CAppStatusLed : public CSysStatusLed {
    bool m_bFanIsActive = false;
    public:
        CAppStatusLed(int nRedPin, int nGreenPin, int nBluePin, bool bActiveLow = true);
        int receiveEvent(const void * pSender, int nMsgId, const void * pMessage, int nType) override;
        unsigned long getNormalBlinkOffTime() override;
        unsigned long getNormalBlinkOnTime() override;
};