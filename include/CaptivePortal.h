#pragma once
#include <DNSServer.h>
#include <ModuleInterface.h>
#include <Appl.h>
#include <LSCUtils.h>


struct CaptivePortalConfig {
    bool    Enabled           = true;    // Enable Captive Portal
    bool    OnAccessPointMode = true;    // Enable Captive Portal in Access Point Mode
    bool    OnStationMode     = false;   // Enable Captive Portal in Station Mode
};

struct CaptivePortalStatus {
    bool    Active         = false;    // Captive Portal is active
    bool    WiFiModeIsAP   = false;    // WiFi is in Access Point Mode or Station Mode  
}; 

class CCaptivePortal : public IModule {
    protected :
        DNSServer *m_pDNSServer;
        IPAddress  m_oRedirectIP;
        CaptivePortalConfig m_oConfig;
        CaptivePortalStatus m_oStatus;

    public:
        CCaptivePortal(bool bAutoRegisterOnMsgBus = true);
        CCaptivePortal*  setup(DNSServer *pDNSServer = nullptr, IPAddress oRedirectIP = (uint32_t)0);
        int receiveEvent(const void * pSender, int nMsg, const void * pMessage, int nClass) override;
        void writeConfigTo(JsonNode &oCfgNode, bool bHideCritical) override;
        void readConfigFrom(JsonNode &oCfgNode) override;
        void writeStatusTo(JsonNode &oStatusNode, int nLevel = STATUS_LEVEL_INFO) override;
        bool startPortal();
        void stopPortal();
        bool isActive() { return m_oStatus.Active; }
        void enable(bool bEnable) {
            m_oConfig.Enabled = bEnable;
        }   
};
