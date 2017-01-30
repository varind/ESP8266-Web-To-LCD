/*
 This version gets rid of many #DEFINE's so you can switch programs on the fly!

 Web to LCD (ESP8266 Only, no Arduino) by www.varind.com 2017. This code is public domain, enjoy!
 Latest code: https://github.com/varind/ESP8266-Web-To-LCD
 Project page: : http://www.variableindustries.com/web-to-lcd-2/

 LCD 1 VSS -> 200 ohm -> +5V
 LCD 2 GND -> GND
 LCD 3 VO Pin -> center pin of 10k Potentiometer (ends to +5V & GND)
 LCD 4 RS pin -> ESP-12  pin 13
 LCD 5 R/W pin -> GND
 LCD 6 Enable pin -> ESP-12 pin 12
 LCD 11 D4 pin -> ESP-12  pin 14
 LCD 12 D5 pin -> ESP-12  pin 5
 LCD 13 D6 pin -> ESP-12  pin 4
 LCD 14 D7 pin -> ESP-12  pin 2
 LCD 15 A (backlight) -> 220 ohm -> +5V
 LCD 16 K (backlight) -> GND

 ESP-12 VCC -> +3.3V
 ESP-12 GND -> GND
 ESP-12 pin CH_PD -> 10K -> +3.3V

 To connect the ESP8266 to your network:
 -Connect your computer to the Wireless AP defined in ESPssid (default: "ESP LCD")
 -Use password set in ESPpassword (default: "PICK A PASSWORD")
 -Use a web browser to go to what is set in apIP (default "192.168.0.1")
 -Click on "cofig wifi settings"
 -Select your home/office access point and enter your password

 To upload new binary to the ESP8266:
 -Connect your computer to the same Wireless AP as the ESP8266
 -Use a web browser and navigate to what is set to host + .local (default "esp.local")
 -Select the new binary to install
 -Click upload
 -Info on mDNS at: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS
 */

#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>

// LiquidCrystal lcd(13, 12, 5, 4, 2, 14); // V1
LiquidCrystal lcd(13, 12, 14, 5, 4, 2); //V2

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
IPAddress apIP(192, 168, 0, 1);
IPAddress netMsk(255, 255, 255, 0);

bool debug = true;

const char *ESPssid = "News LCD";
const char *ESPpassword = "PICK A PASSWORD";
const char* host = "news";
byte lcdRows = 4;
byte lcdCols = 20;

/* Don't set these wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

const char* customBiasElements[64];    // the elements that need custom start/end positions
int customStartBias;          // characters after element start that data begins
int customEndBias;            // characters before end charater that data ends
char dataServer[32];          // base URL of site to connect to
char dataPage[128];           // specific page to connect to
const char* elements[16];       // list of elements to search for
const char *elementPreLabels[16];   // labels to display before data
const char *elementPostLabels[16];    // labels to display after data
char dataEnd[8];            // character that marks the end of data
char elementCols[20];         // formatting for display
int normalStartBias = 0;        // characters after element start that data begins
int normalEndBias = 0;          // characters before end charater that data ends
byte maxDataLength;           // total length of elements plus extra for +IPD reprints
char charBuf[400];            // buffer to hold all data
long reloadDelay;           // time between page reloads

int elementsArrayLength;        // length of array holding all the elements
int customElementsArrayLength; // TODO
char *elementValues[32];

bool longData = false;      // use "longdata" formatting (useful for news)
int instanceNumber = 0;     // find this instance of the element on the page
byte dataInstance;
bool specialReplace = false;        // replace special characters

char customNewLineChar;       // always looks for \n but we may want to find another character
bool bigText = false;           // use big font for data display
bool bigLabels = false;           // use big font for data labels
bool grid = false;              // align to grid
bool customBias = false;        // if certain data elements need custom start/end positions

String line;
byte memPos; // Memory offset

size_t bufArrayLength;

byte char_x = 0, char_y = 0;
uint32_t reloadTimer = 0;
bool jumpStart = true;
bool clientConnected = false;

boolean connect;
uint32_t lastConnectTry = 0;
int status = WL_IDLE_STATUS;

void bbcnews() {
  //----------------------------------------------------------------------------------|
  //                                  BBC NEWS
  // http://feeds.bbci.co.uk/news/rss.xml?edition=us
  //----------------------------------------------------------------------------------|
  char BBCNEWSdataServer[] = "feeds.bbci.co.uk";
  char BBCNEWSdataPage[] = "/news/rss.xml?edition=us";
  const char* BBCNEWSelements[] = { "<title><![CDATA[" };
  const char* BBCNEWSelementPreLabels[] = { "BBC: " };
  const char* BBCNEWSelementPostLabels[] = { "" };
  char BBCNEWSdataEnd[] = "/";
  char BBCNEWSelementCols[] = { 1 };
  normalStartBias = 0;
  normalEndBias = -4;
  maxDataLength = 80;
  reloadDelay = 20000;

  instanceNumber = 3;

  bigText = false;
  bigLabels = false;
  longData = true;
  specialReplace = true;

  setArrays(BBCNEWSdataServer, sizeof(BBCNEWSdataServer), BBCNEWSdataPage,
      sizeof(BBCNEWSdataPage), BBCNEWSelements,
      sizeof(BBCNEWSelements), sizeof(BBCNEWSelements) / sizeof(*BBCNEWSelements),
      BBCNEWSelementPreLabels, sizeof(BBCNEWSelementPreLabels),
      BBCNEWSelementPostLabels, sizeof(BBCNEWSelementPostLabels), BBCNEWSdataEnd,
      sizeof(BBCNEWSdataEnd), BBCNEWSelementCols, sizeof(BBCNEWSelementCols));
}
void nytnews() {
  //----------------------------------------------------------------------------------|
  //                                  NEW YORK TIMES NEWS
  //   ttp://rss.nytimes.com/services/xml/rss/nyt/HomePage.xml
  //----------------------------------------------------------------------------------|
  char NYTNEWSdataServer[] = "rss.nytimes.com";
  char NYTNEWSdataPage[] = "/services/xml/rss/nyt/HomePage.xml";
  const char* NYTNEWSelements[] = { "<title>" };
  const char* NYTNEWSelementPreLabels[] = { "NYT: " };
  const char* NYTNEWSelementPostLabels[] = { "" };
  char NYTNEWSdataEnd[] = "/";
  char NYTNEWSelementCols[] = { 1 };

  normalStartBias = 0;
  normalEndBias = -1;

  instanceNumber = 3;

  bigText = false;
  bigLabels = false;
  longData = true;
  specialReplace = true;

  maxDataLength = 100;
  reloadDelay = 120000;

  setArrays(NYTNEWSdataServer, sizeof(NYTNEWSdataServer), NYTNEWSdataPage,
      sizeof(NYTNEWSdataPage), NYTNEWSelements,
      sizeof(NYTNEWSelements), sizeof(NYTNEWSelements) / sizeof(*NYTNEWSelements),
      NYTNEWSelementPreLabels, sizeof(NYTNEWSelementPreLabels),
      NYTNEWSelementPostLabels, sizeof(NYTNEWSelementPostLabels), NYTNEWSdataEnd,
      sizeof(NYTNEWSdataEnd), NYTNEWSelementCols, sizeof(NYTNEWSelementCols));
}
void abcnews() {
  //----------------------------------------------------------------------------------|
  //                                  ABC NEWS
  //  http://feeds.abcnews.com/abcnews/topstories
  //----------------------------------------------------------------------------------|
  char ABCNEWSdataServer[] = "feeds.abcnews.com";
  char ABCNEWSdataPage[] = "/abcnews/topstories";
  const char* ABCNEWSelements[] = { "<title><![CDATA[" };
  const char* ABCNEWSelementPreLabels[] = { "ABC: " };
  const char* ABCNEWSelementPostLabels[] = { "" };
  char ABCNEWSdataEnd[] = "]";
  char ABCNEWSelementCols[] = { 1 };

  maxDataLength = 80;
  normalStartBias = 0;
  normalEndBias = 0;
  reloadDelay = 120000;

  instanceNumber = 1;
  bigText = false;
  bigLabels = false;
  longData = true;
  specialReplace = true;

  setArrays(ABCNEWSdataServer, sizeof(ABCNEWSdataServer), ABCNEWSdataPage,
      sizeof(ABCNEWSdataPage), ABCNEWSelements,
      sizeof(ABCNEWSelements), sizeof(ABCNEWSelements) / sizeof(*ABCNEWSelements),
      ABCNEWSelementPreLabels, sizeof(ABCNEWSelementPreLabels),
      ABCNEWSelementPostLabels, sizeof(ABCNEWSelementPostLabels), ABCNEWSdataEnd,
      sizeof(ABCNEWSdataEnd), ABCNEWSelementCols, sizeof(ABCNEWSelementCols));
}
void ham() {
  //----------------------------------------------------------------------------------|
  //                     SOLAR ELEMENTS FOR AMATEUR RADIO
  //     http://www.hamqsl.com/solarxml.php
  //----------------------------------------------------------------------------------|
  char HAMdataServer[] = "www.hamqsl.com";
  char HAMdataPage[] = "/solarxml.php";

  const char* HAMelements[] = { "<solarflux>", "<sunspots>", "<aindex>", "<kindex>", "<xray>",
      "<geomagfield>", "<aurora>", "<signalnoise>", "<muf>" };
  const char* HAMelementPreLabels[] = { "SFI:", "SN:", "A:", "K:", "XRY:", "Mag:", "Aur:", "S/N:",
      "MUF:" };

  const char* HAMelementPostLabels[] = { " ", " ", " ", " ", " ", " ", " ", " ", " " };
  char HAMdataEnd[] = "/";
  char HAMelementCols[] = { 3, 2, 2, 2 };

  maxDataLength = 10;
  reloadDelay = 60000;
  normalStartBias = 0;
  normalEndBias = -1;

  instanceNumber = 0;
  bigText = false;
  bigLabels = false;
  longData = false;
  specialReplace = false;

  setArrays(HAMdataServer, sizeof(HAMdataServer), HAMdataPage,
      sizeof(HAMdataPage), HAMelements,
      sizeof(HAMelements), sizeof(HAMelements) / sizeof(*HAMelements),
      HAMelementPreLabels, sizeof(HAMelementPreLabels),
      HAMelementPostLabels, sizeof(HAMelementPostLabels), HAMdataEnd,
      sizeof(HAMdataEnd), HAMelementCols, sizeof(HAMelementCols));
}
void ham2() {
  //----------------------------------------------------------------------------------|
  //                     AMATEUR RADIO BAND INFORMATION
  //    http://www.hamqsl.com/solarxml.php
  //----------------------------------------------------------------------------------|
  char HAM2dataServer[] = "www.hamqsl.com";
  char HAM2dataPage[] = "/solarxml.php";

  const char* HAM2elements[] = { "40m\" time=\"day\">", "40m\" time=\"night\">",
      "20m\" time=\"day\">", "20m\" time=\"night\">", "15m\" time=\"day\">",
      "15m\" time=\"night\">", "10m\" time=\"day\">", "10m\" time=\"night\">" };
  const char* HAM2elementPreLabels[] = { "80-40m D ", "N ", "30-20m D ", "N ", "17-15m D ", "N ",
      "12-10m D ", "N " };

  const char* HAM2elementPostLabels[] = { " ", "", " ", "", " ", "", " ", "" };
  char HAM2dataEnd[] = "/";
  char HAM2elementCols[] = { 2, 2, 2, 2 };

  normalStartBias = 0;
  normalEndBias = -1;
  maxDataLength = 10;
  reloadDelay = 120000;

  instanceNumber = 0;
  bigText = false;
  bigLabels = false;
  longData = false;
  specialReplace = false;

  setArrays(HAM2dataServer, sizeof(HAM2dataServer), HAM2dataPage,
      sizeof(HAM2dataPage), HAM2elements,
      sizeof(HAM2elements), sizeof(HAM2elements) / sizeof(*HAM2elements),
      HAM2elementPreLabels, sizeof(HAM2elementPreLabels),
      HAM2elementPostLabels, sizeof(HAM2elementPostLabels), HAM2dataEnd,
      sizeof(HAM2dataEnd), HAM2elementCols, sizeof(HAM2elementCols));

}
void stocks() {
  //----------------------------------------------------------------------------------|
  //                                  STOCKS
  // http://finance.google.com/finance/info?client=ig&q=MCHP
  //----------------------------------------------------------------------------------|
  char STOCKSdataServer[] = "finance.google.com";
  char STOCKSdataPage[] = "/finance/info?client=ig&q=MCHP";
  const char* STOCKSelements[] = { "\"lt\"", "\"t\"", "\"l_cur\"", "\"c\"", "\"cp\"" };
  const char* STOCKSelementPreLabels[] = { "", "", "$", "", "" };
  const char* STOCKSelementPostLabels[] = { "", ":", "", " ", "%" };
  char STOCKSdataEnd[] = { "\n" };
  normalStartBias = 4;
  normalEndBias = -1;
  char STOCKSelementCols[] = { 1, 2, 2, 1 };
  maxDataLength = 25;
  reloadDelay = 60000;

  instanceNumber = 0;
  bigText = false;
  bigLabels = false;
  longData = false;
  specialReplace = false;

  setArrays(STOCKSdataServer, sizeof(STOCKSdataServer), STOCKSdataPage,
      sizeof(STOCKSdataPage), STOCKSelements,
      sizeof(STOCKSelements), sizeof(STOCKSelements) / sizeof(*STOCKSelements),
      STOCKSelementPreLabels, sizeof(STOCKSelementPreLabels),
      STOCKSelementPostLabels, sizeof(STOCKSelementPostLabels), STOCKSdataEnd,
      sizeof(STOCKSdataEnd), STOCKSelementCols, sizeof(STOCKSelementCols));
}
void bigstocks() {
  //----------------------------------------------------------------------------------|
  //                                 STOCKS (BIG FONT)
  //  http://finance.google.com/finance/info?client=ig&q=MCHP
  //----------------------------------------------------------------------------------|
  char BIGSTOCKSdataServer[] = "finance.google.com";
  char BIGSTOCKSdataPage[] = "/finance/info?client=ig&q=MCHP";
  const char* BIGSTOCKSelements[] = { "\"t\"", "\"l_cur\"" };
  const char* BIGSTOCKSelementPreLabels[] = { "", "" };
  const char* BIGSTOCKSelementPostLabels[] = { "", "" };
  char BIGSTOCKSdataEnd[] = "\n";
  char BIGSTOCKSelementCols[] = { 1, 2 };
  normalStartBias = 4;
  normalEndBias = -1;
  maxDataLength = 8;
  reloadDelay = 60000;

  instanceNumber = 0;
  bigText = true;
  bigLabels = false;
  longData = false;
  specialReplace = false;

  setArrays(BIGSTOCKSdataServer, sizeof(BIGSTOCKSdataServer), BIGSTOCKSdataPage,
      sizeof(BIGSTOCKSdataPage), BIGSTOCKSelements,
      sizeof(BIGSTOCKSelements), sizeof(BIGSTOCKSelements) / sizeof(*BIGSTOCKSelements),
      BIGSTOCKSelementPreLabels, sizeof(BIGSTOCKSelementPreLabels),
      BIGSTOCKSelementPostLabels, sizeof(BIGSTOCKSelementPostLabels), BIGSTOCKSdataEnd,
      sizeof(BIGSTOCKSdataEnd), BIGSTOCKSelementCols, sizeof(BIGSTOCKSelementCols));
}
void weather() {
  //----------------------------------------------------------------------------------|
  //                                  WEATHER
  //   http://w1.weather.gov/xml/current_obs/KPDX.xml"
  //----------------------------------------------------------------------------------|
  char WEATHERdataServer[] = "w1.weather.gov";
  char WEATHERdataPage[] = "/xml/current_obs/KPDX.xml";
  const char* WEATHERelements[] = { "temp_f", "relative_humidity", "pressure_in", "wind_dir",
      "wind_mph", "<weather" };
  const char* WEATHERelementPreLabels[] = { "", " ", "Pressure: ", "", " @ ", "" };
  const char* WEATHERelementPostLabels[] = { "F ", "% Humidity", "in", "", "MPH", "" };
  char WEATHERdataEnd[] = "/";
  char WEATHERelementCols[] = { 2, 1, 2, 1 };

  normalStartBias = 1;
  normalEndBias = -1;
  maxDataLength = 21;
  reloadDelay = 60000;

  instanceNumber = 0;
  bigText = false;
  bigLabels = false;
  longData = false;
  specialReplace = false;

  setArrays(WEATHERdataServer, sizeof(WEATHERdataServer), WEATHERdataPage,
      sizeof(WEATHERdataPage), WEATHERelements,
      sizeof(WEATHERelements), sizeof(WEATHERelements) / sizeof(*WEATHERelements),
      WEATHERelementPreLabels, sizeof(WEATHERelementPreLabels),
      WEATHERelementPostLabels, sizeof(WEATHERelementPostLabels), WEATHERdataEnd,
      sizeof(WEATHERdataEnd), WEATHERelementCols, sizeof(WEATHERelementCols));
}
void bigweather() {
  //----------------------------------------------------------------------------------|
  //                                  WEATHER  (BIG FONT)
  //            http://w1.weather.gov/xml/current_obs/KPDX.xml
  //----------------------------------------------------------------------------------|
  char BIGWEATHERdataServer[] = "w1.weather.gov";
  char BIGWEATHERdataPage[] = "/xml/current_obs/KPDX.xml";
  const char* BIGWEATHERelements[] = { "temp_f", "relative_humidity", "wind_mph" };
  const char* BIGWEATHERelementPreLabels[] = { "", "", "" };
  const char* BIGWEATHERelementPostLabels[] = { "F ", "%", "MPH" };
  char BIGWEATHERdataEnd[] = "/";
  char BIGWEATHERelementCols[] = { 2, 1 };

  normalStartBias = 1;
  normalEndBias = -1;
  maxDataLength = 5;
  reloadDelay = 60000;

  instanceNumber = 0;
  bigText = true;
  bigLabels = false;
  longData = false;
  specialReplace = false;


  setArrays(BIGWEATHERdataServer, sizeof(BIGWEATHERdataServer), BIGWEATHERdataPage,
      sizeof(BIGWEATHERdataPage), BIGWEATHERelements,
      sizeof(BIGWEATHERelements), sizeof(BIGWEATHERelements) / sizeof(*BIGWEATHERelements),
      BIGWEATHERelementPreLabels, sizeof(BIGWEATHERelementPreLabels),
      BIGWEATHERelementPostLabels, sizeof(BIGWEATHERelementPostLabels), BIGWEATHERdataEnd,
      sizeof(BIGWEATHERdataEnd), BIGWEATHERelementCols, sizeof(BIGWEATHERelementCols));

}

void setArrays(char dataServerArray[], uint8_t dsaSize, char dataPageArray[], uint8_t dpaSize,
    const char* elementsArray[], uint8_t eaSize, uint8_t ealSize, const char* elementPreLabelsArray[],
    uint8_t epaSize, const char* elementPostLabelsArray[], uint8_t eplSize, char dataEndArray[],
    uint8_t deaSize, char elementColsArray[], uint8_t ecaSize) {

  strncpy(dataServer, dataServerArray, dsaSize);
  dataServer[dsaSize - 1] = '\0';
  strncpy(dataPage, dataPageArray, dpaSize);
  dataPage[dpaSize - 1] = '\0';
  memcpy(elements, elementsArray, eaSize);
  elementsArrayLength = ealSize;
  memcpy(elementPreLabels, elementPreLabelsArray, epaSize);
  memcpy(elementPostLabels, elementPostLabelsArray, eplSize);
  strncpy(dataEnd, dataEndArray, deaSize);
  dataEnd[deaSize - 1] = '\0';
  memcpy(elementCols, elementColsArray, ecaSize);

}

//----------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(10);
  lcd.begin(lcdCols, lcdRows);
  lcd.clear();

  createChars();                            // for large font

  lcd.setCursor(0, 0);
  lcd.print("    STARTING UP    ");

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ESPssid, ESPpassword);
  delay(500); // Without delay I've seen the IP address blank

  /* Setup web pages: root, wifi config pages, and not found. */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/output", handleOutput);
  server.onNotFound(handleNotFound);

  httpUpdater.setup(&server);
  server.begin(); // Web server start

  Serial.println(">> HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
}

bool startup = true;

unsigned long modeTimer = millis();

typedef void (*modeSelect)();

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
modeSelect mode[] = { weather, bigweather, stocks, bigstocks, ham, ham2, abcnews, bbcnews,nytnews };

int currentModeNumber = 0;
void loop() {
  if (startup == true) {
    mode[0]();
    startup = false;
  }

  if (modeTimer + 10000 < millis()) {
    currentModeNumber = (currentModeNumber + 1) % ARRAY_SIZE(mode); // add 1 and wrap around
    mode[currentModeNumber]();
    reloadTimer = millis() - reloadDelay;
    modeTimer = millis();
  }

  connectRequest();
  server.handleClient();
  getAndDisplay();
}

void getAndDisplay() {
  if (WiFi.status() == WL_CONNECTED) {
    if (reloadTimer + reloadDelay < millis() || jumpStart) {
      getData();
      if (clientConnected) {
        printDataToLCD();
      } else {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("CLIENT DISCONNECTED");
        delay(50);
      }
      reloadTimer = millis();
      jumpStart = false;
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(" WIFI DISCONNECTED ");
    delay(50);
  }
}

void getData() {
  WiFiClient client;
  Serial.print(">> Connecting to ");
  Serial.println(dataServer);
  if (!client.connect(dataServer, 80)) {
    Serial.println(">> Connection Failed !");
    clientConnected = false;
    client.stop();
    return;
  } else {
    clientConnected = true;
  }

  Serial.print(">> Requesting URL: ");
  Serial.println(dataPage);

  Serial.println("");
  client.print(
      String("GET ") + dataPage + " HTTP/1.1\r\n" + "Host: " + dataServer
          + "\r\nUser-Agent: Mozilla/4.0\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">> Client Timeout !");
      client.stop();
      return;
    }
  }

  memPos = 0;
  dataInstance = 0;
  while (client.available()) {

    if (instanceNumber > 0) {
      if (dataInstance == instanceNumber) {
        client.flush();
        dataInstance = 0;
        break;
      }
    }

    line = client.readStringUntil('\n');

    if (specialReplace == true) {
      line.replace("&#x2019;", "\'");         //replace special characters
      line.replace("&#39;", "\'");
      line.replace("&apos;", "\'");
      line.replace("’", "\'");
      line.replace("‘", "\'");
      line.replace("&amp;", "&");
      line.replace("&quot;", "\"");
      line.replace("&gt;", ">");
      line.replace("&lt;", "<");
    }

    int elementPos;
    int elementLength;
    int endElement;
    int startBias, endBias;

    Serial.println(line);

    for (byte i = 0; i < elementsArrayLength; i++) {
      elementLength = strlen(elements[i]);
      elementPos = line.indexOf(elements[i]);
      endElement = line.indexOf(dataEnd);

      if (elementPos > -1) {                       // Found the element!!!

        dataInstance++;

        if (customBias == true) {               // Check for custom bias
          bool customFlag = false;
          for (byte k = 0; k < customElementsArrayLength; k++) {
            if (elements[i] == customBiasElements[k]) {
              startBias = customStartBias;
              endBias = customEndBias;
              customFlag = true;
              break;
            }
          }
          if (!customFlag) {
            startBias = normalStartBias;
            endBias = normalEndBias;
          }
        } else {
          startBias = normalStartBias;
          endBias = normalEndBias;
        }
        int lineLength = line.length();
        if (endElement == -1) {
          if (debug)
            Serial.println(F(">> Can't find end element: Using line end"));
          //   int endPos = endElement + endBias;
          line = line.substring(elementPos + elementLength + startBias,
              lineLength + endBias);
        } else {
          line = line.substring(elementPos + elementLength + startBias,
              endElement + endBias);
        }
        if (line == "")
          line = "-";        // if no value, replace with a dash
        line.toCharArray(charBuf + memPos, maxDataLength);
        elementValues[i] = charBuf + memPos;
        memPos = memPos + line.length() + 1;

        if (debug) {
          Serial.print(">> FOUND ");
          Serial.print(elements[i]);
          Serial.print("  VALUE=");
          Serial.print(elementValues[i]);
          Serial.print("  AT=");
          Serial.print(elementPos);
          Serial.print("  LENGTH=");
          Serial.println(line.length());

//          Serial.println("");
//          Serial.println(elementPos + elementLength + startBias);
//          Serial.println(endElement + endBias);
//          Serial.println("");

          Serial.print(">> MEMORY BUF From ");
          Serial.print(memPos - line.length() - 1);
          Serial.print(" To ");
          Serial.println(memPos);
        }
      }
    }
    line = ""; // to flush the value.
  }
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}

void printDataToLCD() {
  if (debug)
    Serial.println("<---------PRINT DATA TO LCD--------->");
  byte x = 0;
  byte y = 0;
  byte cols = 0;
  byte dataLength = 0;

  byte longDataLength = 0;
  char text[maxDataLength + 20];   // +20 for labels
  text[0] = 0;

  for (byte el = 0; el < elementsArrayLength; el++) {
    dataLength = 0;
    lcd.setCursor(x, y);
    if (elementCols[cols] == 0)
      elementCols[cols] = 1; /////////////!!!!!! TODO what is this about??
    for (byte col = 0; col < elementCols[cols]; col++) {
      if (y == lcdRows)
        break;     // we are off the screen, just stop

      dataLength = dataLength + strlen(elementPreLabels[el]) + strlen(elementValues[el])
          + strlen(elementPostLabels[el]);

      if (grid == true) {
        x = col * ((lcdCols / elementCols[cols]) + 1);
        lcd.setCursor(x, y);
      }

      if (longData == true) {
        longDataLength = longDataLength + dataLength;
        byte offset;
        byte charNum;
        strcat(text, elementPreLabels[el]);
        strcat(text, elementValues[el]);
        strcat(text, elementPostLabels[el]);

        if (el == elementsArrayLength - 1) {
          if (longDataLength > lcdCols * lcdRows)
            longDataLength = lcdCols * lcdRows + 3;
          offset = 0;
          for (byte rowNum = 0; rowNum < lcdRows; rowNum++) {
            lcd.setCursor(x, rowNum);
            charNum = rowNum * lcdCols + offset;
            while (charNum < ((rowNum + 1) * lcdCols) + offset) {
              if (charNum - offset == rowNum * lcdCols && charNum < longDataLength
                  && text[charNum] == ' ') {
                charNum++;
                offset++;
                longDataLength--;
              }
              if (charNum - offset >= longDataLength)
                lcd.write(254);
              // if (charNum - offset >= longDataLength) lcd.print("x");
              else if (charNum - offset < longDataLength)
                lcd.print(text[charNum]);
              charNum++;
            }
          }
        }
      }

      if (bigText == true) {
        char_x = x;
        char_y = y;
        if (bigLabels == true) {
          printBigCharacters(elementPreLabels[el], char_x, char_y);
        } else {
          for (byte i = 0; i < strlen(elementPreLabels[el]); i++) {
            lcd.setCursor(char_x + i, char_y);
            lcd.write(254);
          }
        }
        lcd.setCursor(char_x, char_y + 1);
        lcd.print(elementPreLabels[el]);
        char_x = char_x + strlen(elementPreLabels[el]);
        printBigCharacters(elementValues[el], char_x, char_y);
        if (bigLabels == true) {
          printBigCharacters(elementPostLabels[el], char_x, char_y);
        } else {
          for (byte i = 0; i < strlen(elementPostLabels[el]); i++) {
            lcd.setCursor(char_x + i, char_y);
            lcd.write(254);
          }

          lcd.setCursor(char_x, char_y + 1);
          lcd.print(elementPostLabels[el]);
          char_x = char_x + strlen(elementPostLabels[el]);
        }
        x = x + char_x;
      }

      if (bigText == false && longData == false) {

        lcd.print(elementPreLabels[el]);
        lcd.print(elementValues[el]);
        lcd.print(elementPostLabels[el]);
      }

      if (el == elementsArrayLength - 1 || col == elementCols[cols] - 1) {

        if (grid == false && bigText == false) {

          while (dataLength < lcdCols) {
            lcd.write(254);   // comment this to debug
            //lcd.print("x"); // uncomment this debug
            dataLength++;
          }
        }

        if (grid == false && bigText == true) {

          while (char_x < lcdCols) {
            lcd.setCursor(char_x, char_y);
            lcd.write(254);
            lcd.setCursor(char_x, char_y + 1);
            lcd.write(254);
            char_x++;
          }
        }
        x = 0;
        break;
      }
      el++;
    }
    cols++;
    y++;
    if (bigText == true) {
      y++;
    }
  }
  if (y < lcdRows && longData == false) {  //clear last line of display if not filled  TODO ?
    lcd.setCursor(0, lcdRows - 1);
    for (byte q = 0; q < lcdCols; q++) {
      lcd.write(254);
    }
  }
}

void MDNSSetup() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   SETTING UP MDNS  ");
  lcd.setCursor(0, 1);

  int timeout = 0;
  bool MDNSStatus = true;
  while (!MDNS.begin(host)) {
    lcd.print(".");
    Serial.println(">> Error setting up MDNS responder!");
    delay(1000);
    timeout++;
    if (timeout == 20) {
      Serial.println(">> Setting up MDNS failed!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     MDNS FAILED    ");
      MDNSStatus = false;
      delay(500);
      break;
    }
  }
  if (MDNSStatus) {
    MDNS.addService("http", "tcp", 80);
    Serial.println();
    Serial.println(">> HTTPUpdateServer ready!");
    Serial.printf(">> Open http://%s.local/update in your browser\n", host);
    Serial.println("");
  }
}

void connectWifi() {
  Serial.println(">> Connecting as wifi client...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   CONNECTING TO:   "));
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print(">> Connect Status: ");
  Serial.println(connRes);
}

void connectRequest() {
  if (connect) {
    Serial.println(">> Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print(">> Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("      CONNECTED     "));
        lcd.setCursor(0, 1);
        lcd.print(F("IP: "));
        lcd.print(WiFi.localIP());

        Serial.println("");
        Serial.print(">> Connected to ");
        Serial.println(ssid);
        Serial.print(">> IP address: ");
        Serial.println(WiFi.localIP());

        delay(3000);

        MDNSSetup();

      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
  }
}

/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
  }

  Serial.println(">> Recovered credentials:");
  Serial.print(">> SSID: ");
  Serial.println(ssid);
  Serial.print(">> PASS: ");
  Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void handleOutput() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<html><head><meta http-equiv=\"refresh\" content=\"30\" /></head><body>"
      "<h1>Varind LCD Output</h1>");

  for (byte el = 0; el < elementsArrayLength; el++) {
    server.sendContent(
        String() + "<table border=0><tr><td align=right>" + elementPreLabels[el]
            + "</td><td>" + elementValues[el] + "</td><td align=left>"
            + elementPostLabels[el] + "</td></tr>");
  }

  server.sendContent("</body></html>");
  server.client().stop();
}

/** Handle root or redirect to captive portal */
void handleRoot() {

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<html><head></head><body>"
      "<h1>Varind LCD</h1>");
  if (server.client().localIP() == apIP) {
    server.sendContent(String("<p>You are connected through the soft AP: ") + ESPssid + "</p>");
  } else {
    server.sendContent(
        String("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
  server.sendContent("<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
      "<p>Or <a href='/output'>view the output</a>.</p>"
      "</body></html>");
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Wifi config page handler */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<html><head></head><body>"
      "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    server.sendContent(String("<p>You are connected through the soft AP: ") + ESPssid + "</p>");
  } else {
    server.sendContent(
        String("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
  server.sendContent("\r\n<br />"
      "<table><tr><th align='left'>SoftAP config</th></tr>");
  server.sendContent(String() + "<tr><td>SSID " + String(ESPssid) + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.softAPIP()) + "</td></tr>");
  server.sendContent("</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>");
  server.sendContent(String() + "<tr><td>SSID " + String(ssid) + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.localIP()) + "</td></tr>");
  server.sendContent("</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
  Serial.println(">> scan start");
  int n = WiFi.scanNetworks();
  Serial.println(">> scan done");

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      server.sendContent(
          String() + "\r\n<tr><td>SSID " + WiFi.SSID(i)
              + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : " *") + " ("
              + WiFi.RSSI(i) + ")</td></tr>");
    }
  } else {
    server.sendContent(String() + "<tr><td>No WLAN found</td></tr>");
  }
  server.sendContent("</table>"
      "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
      "<input type='text' placeholder='network' name='n'/>"
      "<br /><input type='password' placeholder='password' name='p'/>"
      "<br /><input type='submit' value='Connect/Disconnect'/></form>"
      "<p>You may want to <a href='/'>return to the home page</a>.</p>"
      "</body></html>");
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println(">> wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

//---------------------- Large LCD Characters ------------------------//
//  Originally by Michael Pilcher, modified by Ben Lipsey
//  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1265696343

byte UB[8] = //1
    {
    B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000 };
byte RT[8] = //2
    {
    B11100,
    B11110,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111 };
byte LL[8] = //3
    {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B01111,
    B00111 };
byte LB[8] = //4
    {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111 };
byte LR[8] = //5
    {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11110,
    B11100 };
byte UMB[8] =  //6
    {
    B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111 };
byte LMB[8] =  //7
    {
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111 };
byte LT[8] =  //8
    {
    B00111,
    B01111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111 };

void createChars() {
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, UMB);
  lcd.createChar(7, LMB);
  lcd.createChar(8, LT);
}

void printBigCharacters(const char character[], byte x, byte y) {
  bool foundChar;
  byte charWidth = 0;
  for (byte i = 0; i < strlen(character); i++) {

    charWidth = 0;
    foundChar = false;

    if (character[i] == '0' || character[i] == 'O' || character[i] == 'o') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
    }

    if (character[i] == '1') {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == '2') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(7);
    }

    if (character[i] == '3') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
    }

    if (character[i] == '4') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == '5') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
    }

    if (character[i] == '6') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
    }

    if (character[i] == '7') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(8);
      lcd.write(254);
    }

    if (character[i] == '8') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
    }

    if (character[i] == '9') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == 'A' || character[i] == 'a') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == 'B' || character[i] == 'b') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(6);
      lcd.write(5);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(7);
      lcd.write(2);
    }

    if (character[i] == 'C' || character[i] == 'c') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(1);
      lcd.write(1);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(4);
    }

    if (character[i] == 'D' || character[i] == 'd') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(4);
      lcd.write(5);
    }

    if (character[i] == 'E' || character[i] == 'e') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(7);
      lcd.write(7);
    }

    if (character[i] == 'F' || character[i] == 'f') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(254);
    }

    if (character[i] == 'G' || character[i] == 'g') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(1);
      lcd.write(1);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
    }

    if (character[i] == 'H' || character[i] == 'h') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(4);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == 'I' || character[i] == 'i') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(255);
      lcd.write(1);
      lcd.setCursor(x, y + 1);
      lcd.write(4);
      lcd.write(255);
      lcd.write(4);
    }

    if (character[i] == 'J' || character[i] == 'j') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.write(254);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(4);
      lcd.write(4);
      lcd.write(5);
    }

    if (character[i] == 'K' || character[i] == 'k') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(4);
      lcd.write(5);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(2);
    }

    if (character[i] == 'L' || character[i] == 'l') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(254);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(4);
      lcd.write(4);
    }

    if (character[i] == 'M' || character[i] == 'm') {
      foundChar = true;
      charWidth = 4;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(3);
      lcd.write(5);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == 'N' || character[i] == 'n') {
      foundChar = true;
      charWidth = 4;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(2);
      lcd.write(254);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(3);
      lcd.write(5);
    }

    if (character[i] == 'P' || character[i] == 'p') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(254);
    }

    if (character[i] == 'Q' || character[i] == 'q') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(1);
      lcd.write(2);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(255);
      lcd.write(4);
    }

    if (character[i] == 'R' || character[i] == 'r') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(255);
      lcd.write(254);
      lcd.write(2);
    }

    if (character[i] == 'S' || character[i] == 's') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(8);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
    }

    if (character[i] == 'T' || character[i] == 't') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(255);
      lcd.write(1);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(255);
      lcd.write(254);
    }

    if (character[i] == 'U' || character[i] == 'u') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(254);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
    }

    if (character[i] == 'V' || character[i] == 'v') {
      foundChar = true;
      charWidth = 4;
      lcd.setCursor(x, y);
      lcd.write(3);
      lcd.write(254);
      lcd.write(254);
      lcd.write(5);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(2);
      lcd.write(8);
      lcd.write(254);
    }

    if (character[i] == 'W' || character[i] == 'w') {
      foundChar = true;
      charWidth = 4;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.write(254);
      lcd.write(254);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(8);
      lcd.write(2);
      lcd.write(5);
    }

    if (character[i] == 'X' || character[i] == 'x') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
      lcd.setCursor(x, y + 1);
      lcd.write(8);
      lcd.write(254);
      lcd.write(2);
    }

    if (character[i] == 'Y' || character[i] == 'y') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(255);
      lcd.write(254);
    }

    if (character[i] == 'Z' || character[i] == 'z') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(6);
      lcd.write(5);
      lcd.setCursor(x, y + 1);
      lcd.write(8);
      lcd.write(7);
      lcd.write(4);
    }

    if (character[i] == '?') {
      foundChar = true;
      charWidth = 3;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(7);
      lcd.write(254);
    }

    if (character[i] == '!') {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
    }
    if (character[i] == ':') {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.print(".");
      lcd.setCursor(x, y + 1);
      lcd.print(".");
    }
    if (character[i] == ' ') {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(254);
    }

    if (character[i] == '.') {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.write(4);
    }
    if (character[i] == '-') {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(4);
      lcd.write(4);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(254);
    }
    if (character[i] == '\"') {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write("\"");
      lcd.setCursor(x, y + 1);
      lcd.write(254);
    }
    if (character[i] == '\'') {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write("\'");
      lcd.setCursor(x, y + 1);
      lcd.write(254);
    }
    if (character[i] == '|') {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write("|");
      lcd.setCursor(x, y + 1);
      lcd.write("|");
    }
    if (character[i] == '/') {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.write("/");
      lcd.setCursor(x, y + 1);
      lcd.write("/");
      lcd.write(254);
    }

    if (foundChar) {
      x = x + charWidth;
      char_x = char_x + charWidth;
    } else {
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.print(character[i]);
      x = x + 1;
      char_x = char_x + 1;
    }
  }
}
