#ifndef DEBUG_LSC_DISPLAY_PAGES
    #undef DEBUGINFOS
#endif

#include <AppConfig.h>
#include <DisplayPages.h>
#include <DevelopmentHelper.h>

#include <Network.h>

void CDisplayAccessPointPage::show(CDisplay *pDisplay) {
  DEBUG_FUNC_START();
    if(pDisplay) {

        pDisplay->setTextColor(COLOR_WHITE);
        pDisplay->setCursor(0, 0);
        pDisplay->setTextSize(2);
        pDisplay->printlnCentered(F(APP_SHORTNAME));
        pDisplay->setTextSize(1);
        pDisplay->println();
        pDisplay->printlnCentered(F("No WiFi...\n"));
        // Arduino WiFi API - available on ESP8266 and ESP32
        pDisplay->print(F("WiFi: "));
        pDisplay->println(WiFi.softAPSSID());
        pDisplay->print(F("http://"));
        pDisplay->println(WiFi.softAPIP());

        pDisplay->display();
        this->isActive = true;
    }
    DEBUG_FUNC_END();
}

void CDisplayLogoPage::show(CDisplay *pDisplay) {
    DEBUG_FUNC_START();
    if(!this->isActive && pDisplay) {
        DEBUG_INFO("Activating LOGO display...");
        pDisplay->setTextColor(COLOR_WHITE);
        pDisplay->setCursor(0, 0);
        pDisplay->setTextSize(2);
        pDisplay->printlnCentered(F(APP_SHORTNAME));
        pDisplay->setTextSize(1);
        pDisplay->println();
        pDisplay->printlnCentered(F("Version " APP_VERSION));
        pDisplay->println();
        pDisplay->printlnCentered(F("(c) " APP_COPYRIGHT " " APP_COMPANY));
        pDisplay->printlnCentered(F(APP_AUTHOR));
        
        pDisplay->display();
        this->isActive = true;
    }
    DEBUG_FUNC_END();
}

void CDisplayDewPointPage::show(CDisplay *pDisplay) {
    DEBUG_FUNC_START();
    if(pDisplay) {
        DEBUG_INFO("Activating DewPoint display...");
        pDisplay->clearDisplay();
        drawPageHeader(pDisplay);
        int16_t nMiddle = pDisplay->width() / 2;
        int16_t nOffsetX = 6;
        int16_t nOffsetY = pDisplay->getCursorY() + 2;
        pDisplay->drawLine(nMiddle,pDisplay->getCursorY(),nMiddle,pDisplay->height(),COLOR_WHITE);
        pDisplay->printAt(nOffsetX +6 ,nOffsetY,F("Indoor"));
        pDisplay->printAt(nMiddle +6 + nOffsetX,nOffsetY,F("Outdoor"));
        nOffsetY += 12;
        if(Status.pSI) {
            char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
            pDisplay->setTextSize(1,2);
            pDisplay->setCursor(0,nOffsetY);
            // Show indoor / outdoor Dew Points
            sprintfSensorDewPoint(szBuffer,Status.pSI);
            pDisplay->printCentered(szBuffer,0,nMiddle);

            // sprintfSensorDewPoint(szBuffer,Status.pSO);
            // pDisplay->printCentered(szBuffer,nMiddle);
 
            nOffsetY += 22; 
            pDisplay->setTextSize(1);
            pDisplay->setCursor(nOffsetX,nOffsetY);
            printSensorTemperature(pDisplay,Status.pSI);
            // pDisplay->setCursor(nMiddle + nOffsetX,nOffsetY);
            // printSensorTemperature(pDisplay,Status.pSO);

            nOffsetY += 8;
            nOffsetX += 12;
            pDisplay->setCursor(nOffsetX,nOffsetY);
            sprintfSensorHumidity(szBuffer,Status.pSI);
            pDisplay->printCentered(szBuffer,0,nMiddle);
            // sprintfSensorHumidity(szBuffer,Status.pSO);
            // pDisplay->printCentered(szBuffer,nMiddle);

            /*
            printSensorHumidity(pDisplay,Status.pSI);
            pDisplay->setCursor(nMiddle + nOffsetX,nOffsetY);
            printSensorHumidity(pDisplay,Status.pSO);
            */
        }
        pDisplay->display();
        this->isActive = true;
    }
    DEBUG_FUNC_END();
      
}

void CDisplayTempPage::show(CDisplay *pDisplay) {
    DEBUG_FUNC_START();
    if(pDisplay) {
        DEBUG_INFO("Activating Temp display (large)...");
        pDisplay->clearDisplay();
        drawPageHeader(pDisplay);
        pDisplay->setTextSize(1,1);
        int16_t nOffsetY = pDisplay->getCursorY() + 2;
        // pDisplay->printAt(nOffsetX,nOffsetY,"Temperature");
        // pDisplay->printAt(nOffsetX,nOffsetY + 12,"Humidity");
        // pDisplay->printAt(nOffsetX,nOffsetY + 24,"Dew Point");

        if(Status.pSI ) {
            char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
            pDisplay->setTextSize(1,1);
            pDisplay->setCursor(0,nOffsetY);
            sprintfSensorHumidity(szBuffer,Status.pSI);
            pDisplay->print(' ');
            pDisplay->print(szBuffer);
            sprintfSensorTemp(szBuffer,Status.pSI);
            pDisplay->printCentered(szBuffer);
            sprintfSensorDewPoint(szBuffer,Status.pSI);
            pDisplay->printRightAligned(szBuffer);
        }

        pDisplay->display();
        this->isActive = true;
    }
    DEBUG_FUNC_END();
      
}

void CDisplayTempPageLarge::show(CDisplay *pDisplay) {
    DEBUG_FUNC_START();
    if(pDisplay) {
        DEBUG_INFO("Activating Temp display...");
        pDisplay->clearDisplay();
        drawPageHeader(pDisplay);
        int16_t nOffsetY = pDisplay->getCursorY() + 2;
        // pDisplay->printAt(nOffsetX,nOffsetY,"Temperature");
        // pDisplay->printAt(nOffsetX,nOffsetY + 12,"Humidity");
        // pDisplay->printAt(nOffsetX,nOffsetY + 24,"Dew Point");

        if(Status.pSI ) {
            char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
            const char szTempScale[] = { (char) TEMPERATURE_SIGN, 'C', '\0' };

            pDisplay->setTextSize(1,1);
            pDisplay->setCursor(0,nOffsetY);
            sprintfSensorHumidity(szBuffer,Status.pSI);
            pDisplay->print(' ');
            pDisplay->print(szBuffer);
            sprintfSensorDewPoint(szBuffer,Status.pSI);
            pDisplay->printRightAligned(szBuffer);

            pDisplay->setTextSize(3,3);
            pDisplay->setCursor(0,nOffsetY + 20);
            sprintfDisplayNumber(szBuffer,Status.pSI->TempC,Format::Temperature);
            int nTextMetricWidth = 0;
            nTextMetricWidth += pDisplay->getTextWidthMetric(szBuffer,   3);
            nTextMetricWidth += pDisplay->getTextWidthMetric(szTempScale,2);
            pDisplay->setCenteredTextCursorStart(nTextMetricWidth);
            pDisplay->setTextSize(3,3);
            pDisplay->print(szBuffer);
            pDisplay->setTextSize(2,2);
            pDisplay->print(szTempScale);
        }

        pDisplay->display();
        this->isActive = true;
    }
    DEBUG_FUNC_END();
      
}

void CDisplayHumidityPageLarge::show(CDisplay *pDisplay) {
    DEBUG_FUNC_START();
    if(pDisplay) {
        DEBUG_INFO("Activating Humidity display (large)...");
        pDisplay->clearDisplay();
        drawPageHeader(pDisplay);
        int16_t nOffsetY = pDisplay->getCursorY() + 2;
        if(Status.pSI ) {
            char szBuffer[DISPLAY_VALUE_BUFFER_SIZE];
            pDisplay->setTextSize(1,1);
            pDisplay->setCursor(0,nOffsetY);
            sprintfSensorTemp(szBuffer,Status.pSI);

            pDisplay->print(szBuffer);
            sprintfSensorDewPoint(szBuffer,Status.pSI);
            pDisplay->printRightAligned(szBuffer);

            pDisplay->setTextSize(3,3);
            pDisplay->setCursor(0,nOffsetY + 20);

            sprintfDisplayNumber(szBuffer,Status.pSI->Humidity,Format::Humidity);
            pDisplay->printCentered(szBuffer);
            pDisplay->setTextSize(1,2);
            pDisplay->print(F(" %"));
        }

        pDisplay->display();
        this->isActive = true;
    }
    DEBUG_FUNC_END();     
}


void CDisplayChartPage::show(CDisplay *pDisplay) {
    
}