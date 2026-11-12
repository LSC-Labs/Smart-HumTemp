/**
  @PageBuilder:register(InfoPage,CInfoPage)
 */

/**
 * Info page - shows the current values of the sensor (temperature, humidity, dew point).
 * The status is requested every 5 seconds while the page is shown.
 */
class CInfoPage extends CPageHandler {
    /**
     * @param {CAPP} oApp 
     */
    constructor(oApp) {
        // don't use this, before initialized !
        super("info",new CElement("#InfoPage"),oApp.Config);
        this._App = oApp;
    }

    // #region Update Status on a regular base
    disposePage(pView, pApp) {
        super.disposePage(pView, pApp);
        this.RefreshTimer = clearInterval(this.RefreshTimer);
        this.RefreshTimer = null;
    }

    preparePage(pView, pApp) {
        super.preparePage(pView, pApp);
        // Set an interval to refresh the values every 5 seconds...
        this.RefreshTimer = setInterval( this.refreshStatus.bind(this, pView, pApp), 5000);
    }
    
    refreshStatus(pView, pApp) {
        pApp.requestActStatus();
    }

    /**
     * Triggered, when a new status message is received from the web socket.
     * @param {CView} pView 
     * @param {CApp} pApp 
     * @returns false to avoid loadPageConfig is called again (!)
     */
    onUpdateStatusReceived(pView, pApp) {
        this.loadPageConfig(pView, pApp);
        return(false)
    }
    // #endregion

    /**
     * Set the values of the device status - the sub nodes with prefix ("sensor_temp_c"...)
     * @param {CView} pView 
     * @param {CApp} pApp 
     * @returns the device status
     * @overload
     */
    loadPageConfig(pView, pApp) {
        let oStatus = pApp.DeviceStatus;
        if(oStatus) {
            this.setConfigValues(pView,oStatus);
            for(let strName in oStatus) {
                let oData = oStatus[strName];
                if(oData && Utils.isObj(oData)) {
                    if(strName == "sensor") oData = this.formatSensorValues(oData);
                    this.setConfigValues(pView,oData,strName);
                }
            }
        }
        pView.translate();
        return(oStatus);
    }

    /**
     * Returns a copy of the sensor status with the measurement values rounded
     * to one decimal place (the device status itself is not modified).
     * @param {object} oSensor sensor node of the device status
     * @returns {object} formatted copy
     */
    formatSensorValues(oSensor) {
        let oResult = Object.assign({}, oSensor);
        ["temp_c", "hum", "dew_c", "abs_hum"].forEach(strKey => {
            let fValue = parseFloat(oResult[strKey]);
            if(!isNaN(fValue)) oResult[strKey] = fValue.toFixed(1);
        });
        return(oResult);
    }
}
