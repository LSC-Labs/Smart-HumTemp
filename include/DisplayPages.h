#pragma once

#include <EventHandler.h>
#include <Display.h>
#include <DisplayPage.h>
#include <DisplayBasePage.h>
#include <DisplayChart.h>


class CDisplayAccessPointPage : public CDisplayPage {
    public:
        CDisplayAccessPointPage(const char *pszName) : CDisplayPage(pszName,PageType::SETUP) {}
        virtual void show(CDisplay *pDisplay) override;
    
};

class CDisplayLogoPage : public CDisplayPage {
    public:
        CDisplayLogoPage(const char *pszName) : CDisplayPage(pszName,PageType::SETUP,10000) {}
        virtual void show(CDisplay *pDisplay) override;
    
};

class CDisplayDewPointPage : public CDisplayBasePage {
    public:
        CDisplayDewPointPage(const char *pszName) : CDisplayBasePage(pszName,PageType::LOOP) {}
        virtual void show(CDisplay *pDisplay) override;
    
};

class CDisplayTempPage : public CDisplayBasePage {
    public:
        CDisplayTempPage(const char *pszName) : CDisplayBasePage(pszName,PageType::LOOP) {}
        virtual void show(CDisplay *pDisplay) override;
};

class CDisplayTempPageLarge : public CDisplayBasePage {
    public:
        CDisplayTempPageLarge(const char *pszName) : CDisplayBasePage(pszName,PageType::LOOP) {}
        virtual void show(CDisplay *pDisplay) override;
};

class CDisplayHumidityPageLarge : public CDisplayBasePage {
    public:
        CDisplayHumidityPageLarge(const char *pszName) : CDisplayBasePage(pszName,PageType::LOOP) {}
        virtual void show(CDisplay *pDisplay) override;
};

class CDisplayChartPage : public CDisplayBasePage {
    protected:
        CDisplayChart m_oTempChart;
    public:
        CDisplayChartPage(const char *pszName) : 
                        CDisplayBasePage(pszName,PageType::LOOP),
                        m_oTempChart(SENSOR_INDOOR) 
                        {
                            m_oTempChart.collectDataType(CDisplayChart::DataType::TEMP);
                        }
        virtual void show(CDisplay *pDisplay) override;
};
