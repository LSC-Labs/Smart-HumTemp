


#include <CaptivePortal.h>
#include <WiFiController.h>
#include <DevelopmentHelper.h>


#define DNS_DEFAULT_PORT 53

CCaptivePortal::CCaptivePortal(bool bAutoRegisterOnMsgBus) {
    if(bAutoRegisterOnMsgBus) {
        Appl.MsgBus.registerEventReceiver(this);
    }
}

void CCaptivePortal::writeConfigTo(JsonObject &oCfgNode, bool bHideCritical) {
    oCfgNode["enabled"] = m_oConfig.Enabled;
    oCfgNode["onAccessPointMode"] = m_oConfig.OnAccessPointMode;
    oCfgNode["onStationMode"] = m_oConfig.OnStationMode;
}

void CCaptivePortal::readConfigFrom(JsonObject &oCfgNode) {
    LSC::setValue(&m_oConfig.Enabled, oCfgNode["enabled"], &m_oConfig.Enabled);
    LSC::setValue(&m_oConfig.OnAccessPointMode, oCfgNode["onAccessPointMode"], &m_oConfig.OnAccessPointMode);
    LSC::setValue(&m_oConfig.OnStationMode, oCfgNode["onStationMode"], &m_oConfig.OnStationMode);
}

void CCaptivePortal::writeStatusTo(JsonObject &oStatusNode) {
    oStatusNode["active"] = m_oStatus.Active;
}

/**
 * @brief Setup the Captive Portal with DNS Server and Redirect IP
 * @param pDNSServer Pointer to an already created DNSServer instance. If nullptr, a new instance will be created.
 * @param oRedirectIP IP Address to which all DNS requests will be redirected. If (uint32_t)0, the SoftAP IP will be used.
 */
CCaptivePortal* CCaptivePortal::setup( DNSServer *pDNSServer, IPAddress oRedirectIP) {
    DEBUG_FUNC_START();
    m_pDNSServer = pDNSServer;
    m_oRedirectIP = oRedirectIP;
    DEBUG_FUNC_END();
    return(this);
}

bool CCaptivePortal::startPortal() {
    DEBUG_FUNC_START();
    bool bResult = false;
    if(m_oConfig.Enabled == false) {
        DEBUG_INFO("Captive Portal: Not enabled, skipping start.");
    } else {
        DEBUG_INFO("Captive Portal: Starting DNS Server");
        // If no redirect IP specified, use the SoftAP IP
        if(m_oRedirectIP == (uint32_t)0) {
            m_oRedirectIP = m_oStatus.WiFiModeIsAP ?  WiFi.softAPIP() : WiFi.localIP();
            // m_oRedirectIP = IPAddress(192,168,4,1); //  WiFi.localIP();
            DEBUG_INFOS("Captive Portal: No redirect IP specified, using own IP %s",m_oRedirectIP.toString().c_str() );
        }   
        if(m_pDNSServer == nullptr) {
            m_pDNSServer = new DNSServer();
        }
        m_pDNSServer->setErrorReplyCode(DNSReplyCode::NoError);
        m_pDNSServer->start(DNS_DEFAULT_PORT, "*", m_oRedirectIP);
        Appl.MsgBus.sendEvent(this,MSG_CAPTIVE_PORTAL_STARTED,nullptr,0);
        m_oStatus.Active = true;
        bResult = true;
    }
    DEBUG_FUNC_END();
    return bResult;
}

void CCaptivePortal::stopPortal() {
    DEBUG_FUNC_START();
    if(m_pDNSServer != nullptr && m_oStatus.Active) {
        DEBUG_INFO("Captive Portal: Stopping DNS Server");
        // TODO: Check if it is possible to remove the redirection instead of stopping the server
        m_pDNSServer->stop();
        Appl.MsgBus.sendEvent(this,MSG_CAPTIVE_PORTAL_STOPPED,nullptr,0);
        m_oStatus.Active = false;
    }
    DEBUG_FUNC_END();
}

int CCaptivePortal::receiveEvent(const void * pSender, int nMsg, const void * pMessage, int nClass) {
    switch(nMsg) {
        case MSG_WIFI_CONNECTED:
            DEBUG_INFO("Captive Portal: WiFi started - evaluating portal start conditions");
            if(m_oConfig.Enabled ) {       
                if(
                    (nClass == WIFI_ACCESS_POINT_MODE && m_oConfig.OnAccessPointMode)  ||
                    (nClass == WIFI_STATION_MODE      && m_oConfig.OnStationMode ) 
                ) {
                    m_oStatus.WiFiModeIsAP = (nClass == WIFI_ACCESS_POINT_MODE);
                    startPortal();
                }
            }
            break;

        case MSG_WIFI_DISABLED:
            DEBUG_INFO("Captive Portal: WiFi stopped - stopping DNS Server");
            if(m_oStatus.Active) stopPortal();
            break;

        case MSG_APPL_LOOP:
            if(m_pDNSServer != nullptr) {
                m_pDNSServer->processNextRequest();
            }
            break;
    }
    return(0);
}