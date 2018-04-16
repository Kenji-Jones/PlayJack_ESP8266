/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>

extern "C" {
  #include "user_interface.h"
}
const char HTTP_LOGO[] PROGMEM            = "<section class='logo' ></section>";
const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;}  h1 {text-align: center; background-color: green; color: white;} h3 {text-align: center; font-size:1.5rem;padding-top:0; margin-top: 0;} h5 {text-align: right; font-size:0.8rem; padding-top:0; margin-top: 0;} h4 {width 60%; padding: 0; margin: 0;text-align: left; font-size:1.2rem;} label{padding: 5px; padding-bottom:15px;font-size:1.1em; font-weight: bold;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;} .logo {width: 80%; margin-left: auto; margin-right: auto; background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAW4AAAA9CAYAAACNz4NiAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyhpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuNi1jMTM4IDc5LjE1OTgyNCwgMjAxNi8wOS8xNC0wMTowOTowMSAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENDIDIwMTcgKE1hY2ludG9zaCkiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6Q0VDMTU4NkY2OUVFMTFFN0JEMUVEMUJEMUM1ODRBMTMiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6Q0VDMTU4NzA2OUVFMTFFN0JEMUVEMUJEMUM1ODRBMTMiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDpDRUMxNTg2RDY5RUUxMUU3QkQxRUQxQkQxQzU4NEExMyIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDpDRUMxNTg2RTY5RUUxMUU3QkQxRUQxQkQxQzU4NEExMyIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/Pi/5vpcAABV2SURBVHja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\") no-repeat center; background-size: contain; height: 64px} </style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='padding:5px 30px; text-align:left;display:inline-block;min-width:260px; border-style: solid; border-color: green; border-width: medium;'>";
// const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}><br/>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div style = 'font-size: 1.2em;'><span style = 'font-weight: bold;'>Saving Credentials</span><br />Trying to connect device to network.<br />If it fails the <span style ='color: RED;'> RED LED</span> will blink several times. reconnect to the device's wifi to try again<br />If it connects successfully <span style ='color: green;'>GREEN LED</span> will blink several times</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

#define WIFI_MANAGER_MAX_PARAMS 10
#define WFM_LABEL_BEFORE 1
#define WFM_LABEL_AFTER 2
#define WFM_NO_LABEL 0
class WiFiManagerParameter {
  public:
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter
    void          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsucessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;
    char          temp[1500];

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    void          handleSubmit();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    WiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
