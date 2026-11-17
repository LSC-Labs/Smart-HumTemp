#pragma once
#include <Arduino.h>
#include <EventHandler.h>
#include <_DisplayHardware.h>

#ifndef DISPLAY_START_PAGE
    #define DISPLAY_START_PAGE "LOGO"
#endif


class CDisplay;

enum PageType {
    ALL = 0,
    SETUP = 1,
    LOOP  = 2
};

class IDisplayPage
{
    public:
        virtual PageType getType() = 0;
        virtual void show(CDisplay * pDisplay) = 0;
        virtual void hide(CDisplay * pDisplay) = 0;
        virtual bool hasName(const char * pszName) = 0;
        virtual unsigned long getRefreshTime() = 0;
}; 

class CDisplayPage : public IDisplayPage {

    protected:
        // RAM: the name is not copied - it must be a persistent string (i.e. a literal)
        const char *m_pszName;
        unsigned long m_ulRefreshTime = 500;
        PageType    m_eType;
        bool isActive = false;
        
    public:
        CDisplayPage(const char *pszName,   PageType eType = PageType::LOOP, unsigned long ulRefreshTime = 500);
        const char *    getName() { return(m_pszName); };
        PageType        getType() { return(m_eType); };
        void            setRefreshTime(unsigned long ulTime) { m_ulRefreshTime = ulTime; }
        unsigned long   getRefreshTime() { return(m_ulRefreshTime); }
        bool            hasName(const char *pszName);
        virtual void    show(   CDisplay * pDisplay);
        virtual void    hide(   CDisplay * pDisplay);
//         virtual int receiveEvent(const void * pSender, int nMsgType, const void * pMessage, int nClass) override;

    protected:
};