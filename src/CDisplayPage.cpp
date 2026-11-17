
#ifndef DEBUG_LSC_DISPLAY_PAGES
    #undef DEBUGINFOS
#endif

#include <Arduino.h>
#include <DisplayPage.h>
#include <Msgs.h>
#include <Display.h>
#include <DevelopmentHelper.h>

CDisplayPage::CDisplayPage(const char *pszName, PageType eType, unsigned long ulRefreshTime) { 
    m_pszName = pszName ? pszName : "";
    m_eType = eType; 
    setRefreshTime(ulRefreshTime);
};
     

bool CDisplayPage::hasName(const char *pszName) {
    DEBUG_FUNC_START_PARMS("%s",NULL_POINTER_STRING(pszName));
    bool bResult = pszName && strcmp(pszName, m_pszName) == 0;
    DEBUG_FUNC_END_PARMS("%d",bResult);
    return(bResult);
};

void CDisplayPage::show(CDisplay * pDisplay) {
    DEBUG_FUNC_START();
    DEBUG_FUNC_END();
}

void CDisplayPage::hide(CDisplay * pDisplay) {
    if(isActive && pDisplay) {
        pDisplay->clearDisplay();
        isActive = false;
    }
}
