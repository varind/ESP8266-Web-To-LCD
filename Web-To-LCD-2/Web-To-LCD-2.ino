/*
   Web to LCD (ESP8266 Only, no Arduino) by www.varind.com 2016. This code is public domain, enjoy!
   https://github.com/varind/ESP8266-Web-To-LCD
*/

#include <LiquidCrystal.h>
LiquidCrystal lcd(13, 12, 5, 4, 2, 14);
byte lcdRows = 4;
byte lcdCols = 20;

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

bool debug = true;

const char *ESPssid     = "ESP's SSID";
const char *ESPpassword = "ESP's PASS";

const char* host     = "ESPHOST";
const char* ssid     = "YourSSID";
const char* password = "YourPASS";



#define BBCNEWS
//#define NYTNEWS
//#define ABCNEWS
//#define HAM
//#define HAM2
//#define WEATHER
//#define BIGWEATHER
//#define STOCK
//#define BIGSTOCK

//----------------------------------------------------------------------------------|
//                                  BBC NEWS
//----------------------------------------------------------------------------------|
#ifdef BBCNEWS // http://feeds.bbci.co.uk/news/rss.xml?edition=us
char dataServer[] = "feeds.bbci.co.uk";         // base URL of site to connect to
char dataPage[] = "/news/rss.xml?edition=us";   // specific paget to conenct to
const char *elements[]  = {"<title><![CDATA["};          // list of elements to search for
char *elementPreLabels[]  = {"BBC NEWS: "};     // labels to display before data
char *elementPostLabels[]  = {""};              // labels to display after data
const char dataEnd = '/';                       // character that marks the end of data
int normalStartBias = 0;                 // characters after element start that data begins
int normalEndBias = -4;                  // characters before end charater that data ends
const char elementCols[] = {1};                 // formatting for display
#define LONGDATA                                // use "longdata" formatting (useful for news)
#define INSTANCENUMBER 2                        // find this instance of the element on the page
#define SPECIALREPLACE                          // replace special characters
const byte maxDataLength = 80;                  // longest possible length of element data
char charBuf[150];                              // total length of elements plus extra for +IPD reprints
const long reloadDelay = 20000;                // time between page reloads
//#define CUSTOMNEWLINECHAR '\r'                // always looks for \n but we may want to find onother character
//#define BIGTEXT                               // use big font for data display
//#define BIGLABELS                             // use big font for data labels
//#define GRID                                  // align to grid
//#define CUSTOMBIAS                            // if certain data elements need custom start/end postitions
#ifdef CUSTOMBIAS
const char *customBiasElements[] = {""};        // the elements that need custom start/end positions
int customStartBias = 0;                 // characters after element start that data begins
int customEndBias = 0;                   // characters before end charater that data ends
#endif
#endif
//----------------------------------------------------------------------------------|
//                                  NEW YORK TIMES NEWS
//----------------------------------------------------------------------------------|
#ifdef NYTNEWS // http://rss.nytimes.com/services/xml/rss/nyt/HomePage.xml
char dataServer[] = "rss.nytimes.com";
char dataPage[] = "/services/xml/rss/nyt/HomePage.xml";
const char *elements[]  = {"<title>"};
char *elementPreLabels[]  = {"BREAKING: "};
char *elementPostLabels[]  = {""};
const char dataEnd = '/';
int normalStartBias = 0;
int normalEndBias = -1;
const char elementCols[] = {1};

#define LONGDATA
#define INSTANCENUMBER 3
#define SPECIALREPLACE
const byte maxDataLength = 100;
char charBuf[200];
const long reloadDelay = 120000;
#endif

//----------------------------------------------------------------------------------|
//                                  ABC NEWS
//----------------------------------------------------------------------------------|
#ifdef ABCNEWS // http://feeds.abcnews.com/abcnews/topstories
char dataServer[] = "feeds.abcnews.com";
char dataPage[] = "/abcnews/topstories";
const char *elements[]  = {"<title><![CDATA["};
char *elementPreLabels[]  = {"BREAKING: "};
char *elementPostLabels[]  = {""};
const char dataEnd = ']';
int normalStartBias = 0;
int normalEndBias = 0;
const char elementCols[] = {1};

#define LONGDATA
#define INSTANCENUMBER 1
#define SPECIALREPLACE
const byte maxDataLength = 80;
char charBuf[80];
const long reloadDelay = 120000;
#endif
//----------------------------------------------------------------------------------|
//                     SOLAR ELEMENTS FOR AMATEUR RADIO
//----------------------------------------------------------------------------------|
#ifdef HAM // http://www.hamqsl.com/solarxml.php
char dataServer[] = "www.hamqsl.com";
char dataPage[] = "/solarxml.php";

const char *elements[]  = {"<solarflux>", "<sunspots>", "<aindex>",
                           "<kindex>", "<xray>", "<geomagfield>",
                           "<aurora>", "<signalnoise>", "<muf>"
                          };
char *elementPreLabels[]  = {"SFI:", "SN:", "A:",
                             "K:",  "XRY:", "Mag:",
                             "Aur:", "S/N:", "MUF:"
                            };

char *elementPostLabels[]  = {" ", " ", " ", " ", " ", " ", " ", " ", " "};
const char dataEnd = '/';
int normalStartBias = 0;
int normalEndBias = -1;
const char elementCols[] = {3, 2, 2, 2};
const byte maxDataLength = 10;
char charBuf[100];
long reloadDelay = 60000;
#endif
//----------------------------------------------------------------------------------|
//                     AMATEUR RADIO BAND INFORMATION
//----------------------------------------------------------------------------------|
#ifdef HAM2 // http://www.hamqsl.com/solarxml.php
char dataServer[] = "www.hamqsl.com";
char dataPage[] = "/solarxml.php";

const char *elements[]  = {"40m\" time=\"day\">", "40m\" time=\"night\">",
                           "20m\" time=\"day\">", "20m\" time=\"night\">",
                           "15m\" time=\"day\">", "15m\" time=\"night\">",
                           "10m\" time=\"day\">", "10m\" time=\"night\">"
                          };
char *elementPreLabels[]  = {"80-40m D ", "N ",
                             "30-20m D ", "N ",
                             "17-15m D ", "N ",
                             "12-10m D ", "N "
                            };

char *elementPostLabels[]  = {" ", "", " ", "", " ", "", " ", ""};
const char dataEnd = '/';
int normalStartBias = 0;
int normalEndBias = -1;
const char elementCols[] = {2, 2, 2, 2};
const byte maxDataLength = 10;
char charBuf[50];
long reloadDelay = 120000;
#endif
//----------------------------------------------------------------------------------|
//                                  STOCKS
//----------------------------------------------------------------------------------|
#ifdef STOCK  // http://finance.google.com/finance/info?client=ig&q=MCHP
char dataServer[] = "finance.google.com";
char dataPage[] = "/finance/info?client=ig&q=MCHP";
const char *elements[]  = {"\"lt\"", "\"t\"", "\"l_cur\"", "\"c\"", "\"cp\""};
char *elementPreLabels[]  = {"", "", "$", "", ""};
char *elementPostLabels[]  = {"", ":", "", " ", "%"};
const char dataEnd = '\0';
int normalStartBias = 4;
int normalEndBias = -2;
const char elementCols[] = {1, 2, 2, 1};
const byte maxDataLength = 25;
char charBuf[50];
long reloadDelay = 60000;
#endif
//----------------------------------------------------------------------------------|
//                                  STOCKS (BIG FONT)
//----------------------------------------------------------------------------------|
#ifdef BIGSTOCK  // http://finance.google.com/finance/info?client=ig&q=MCHP
char dataServer[] = "finance.google.com";
char dataPage[] = "/finance/info?client=ig&q=MCHP";
const char *elements[]  = {"\"t\"", "\"l_cur\""};
char *elementPreLabels[]  = {"", ""};
char *elementPostLabels[]  = {"", ""};
const char dataEnd = '\n';
int normalStartBias = 4;
int normalEndBias = -1;
const char elementCols[] = {1, 2};
#define BIGTEXT
const byte maxDataLength = 8;
char charBuf[50];
long reloadDelay = 60000;
#endif
//----------------------------------------------------------------------------------|
//                                  WEATHER
//----------------------------------------------------------------------------------|
#ifdef WEATHER // http://w1.weather.gov/xml/current_obs/KPDX.xml"
char dataServer[] = "w1.weather.gov";
char dataPage[] = "/xml/current_obs/KPDX.xml";
const char *elements[]  = {"temp_f", "relative_humidity", "pressure_in", "wind_dir", "wind_mph", "<weather"};
char *elementPreLabels[]  = {"", " ", "Pressure: ", "", " @ ", ""};
char *elementPostLabels[]  = {"F ", "% Humidity", "in", "", "MPH", ""};
const char dataEnd = '/';
int normalStartBias = 1;
int normalEndBias = -1;
const char elementCols[] = {2, 1, 2, 1};
const byte maxDataLength = 21;
char charBuf[50];
long reloadDelay = 60000;
#endif
//----------------------------------------------------------------------------------|
//                                  WEATHER  (BIG FONT)
//----------------------------------------------------------------------------------|
#ifdef BIGWEATHER // http://w1.weather.gov/xml/current_obs/KPDX.xml"
char dataServer[] = "w1.weather.gov";
char dataPage[] = "/xml/current_obs/KPDX.xml";
const char *elements[]  = {"temp_f", "relative_humidity", "wind_mph"};
char *elementPreLabels[]  = {"", "", ""};
char *elementPostLabels[]  = {"F ", "%", "MPH"};
const char dataEnd = '/';
int normalStartBias = 1;
int normalEndBias = -1;
const char elementCols[] = {2, 1};
#define BIGTEXT
//#define BIGLABELS
const byte maxDataLength = 5;
char charBuf[20];
long reloadDelay = 60000;
#endif
//----------------------------------------------------------------------------------


String line;
byte memPos = 0; // Memory offset
const byte elementsArrayLength = sizeof(elements) / sizeof(*elements);
const byte colArrayLength = sizeof(elementCols) / sizeof(*elementCols);

char *elementValues[elementsArrayLength + 1];
byte char_x = 0, char_y = 0;
long timer = 0;
bool jumpStart = true;

#ifdef CUSTOMBIAS
const byte customElementsArrayLength = sizeof(customBiasElements) / sizeof(*customBiasElements);
#endif
#ifdef INSTANCENUMBER
byte dataInstance = 0;
#endif



void setup() {
  Serial.begin(115200);
  delay(10);
  lcd.begin(lcdCols, lcdRows);
  lcd.clear();
#ifdef BIGTEXT
  createChars();                            // for large font
#endif

  lcd.setCursor(0, 0);
  lcd.print(F("    JOINING AP:"));
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.softAP(ESPssid, ESPpassword);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("      CONNECTED     "));
  lcd.setCursor(0, 1);
  lcd.print(F("IP: "));
  lcd.print(WiFi.localIP());

  Serial.println();
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  httpServer.handleClient();
  if (timer + reloadDelay < millis() || jumpStart) {
    getData();
    printDataToLCD();
    timer = millis();
    jumpStart = false;
  }
}


void getData() {
  WiFiClient client;
  Serial.print(F("connecting to "));
  Serial.println(dataServer);
  if (!client.connect(dataServer, 80)) {
    Serial.println(F(">> Connection Failed !"));
    return;
  }

  String url = dataPage;

  Serial.print(F(">> Requesting URL: "));
  Serial.println(dataPage);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + dataServer + "\r\nUser-Agent: Mozilla/4.0\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">> Client Timeout !");
      client.stop();
      return;
    }
  }

  memPos = 0;
  while (client.available()) {

#ifdef INSTANCENUMBER
    if (dataInstance == INSTANCENUMBER) {
      client.flush();
      dataInstance = 0;
      break;
    }
#endif

    line = client.readStringUntil('\n');

#ifdef SPECIALREPLACE
    line.replace("&#x2019;", "\'");                        //replace special characters
    line.replace("&#39;", "\'");
    line.replace("&apos;", "\'");
    line.replace("’", "\'");
    line.replace("‘", "\'");
    line.replace("&amp;", "&");
    line.replace("&quot;", "\"");
    line.replace("&gt;", ">");
    line.replace("&lt;", "<");
#endif

    int elementPos;
    int elementLength;
    int endElement;
    int startBias, endBias;

    Serial.println(line);

    for (byte i = 0; i < elementsArrayLength; i++) {
      elementLength = strlen (elements[i]);
      elementPos = line.indexOf(elements[i]);
      endElement = line.indexOf(dataEnd);

      if (elementPos > -1)  {                                       // Found the element!!!

#ifdef INSTANCENUMBER
        dataInstance++;
#endif

#ifdef CUSTOMBIAS                                  // Check for custom bias
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
#else
        startBias = normalStartBias;
        endBias = normalEndBias;
#endif
        int lineLength = line.length();
        if (endElement == -1) {
          if (debug)Serial.println(F(">> Can't find end element: Using line end"));
          int endPos = endElement + endBias;
          line = line.substring(elementPos + elementLength + startBias, lineLength + endBias);
        } else {
          line = line.substring(elementPos + elementLength + startBias, endElement + endBias);
        }
        if (line == "")line = "-";        // if no value, replace with a dash
        line.toCharArray(charBuf + memPos, maxDataLength);
        elementValues[i] = charBuf + memPos;
        memPos = memPos + line.length() + 1;

        if (debug) {
          Serial.print(F(">> FOUND "));
          Serial.print(elements[i]);
          Serial.print(F("  VALUE="));
          Serial.print(elementValues[i]);
          Serial.print(F("  LENGTH="));
          Serial.println(line.length());

          Serial.print(F(">> MEMORY BUF From "));
          Serial.print(memPos - line.length() - 1);
          Serial.print(F(" To "));
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
  if (debug) Serial.println(F("<---------PRINT DATA TO LCD--------->"));
  byte x = 0;
  byte y = 0;
  byte cols = 0;
  byte dataLength = 0;

#ifdef LONGDATA
  byte longDataLength = 0;
  char text[maxDataLength + 20];   // +20 for labels
  text[0] = 0;
#endif

  for (byte el = 0; el < elementsArrayLength; el++) {
    dataLength = 0;
    lcd.setCursor(x, y);
    for (byte col = 0; col < elementCols[cols]; col++) {
      if (y == lcdRows) break;     // we are off the screen, just stop

      dataLength = dataLength + strlen (elementPreLabels[el]) + strlen (elementValues[el]) + strlen (elementPostLabels[el]);

#ifdef GRID
      x = col * ((lcdCols / elementCols[cols]) + 1);
      lcd.setCursor(x, y);
#endif


#ifdef LONGDATA
      longDataLength = longDataLength + dataLength;
      byte offset;
      byte charNum;
      strcat(text, elementPreLabels[el]);
      strcat(text, elementValues[el]);
      strcat(text, elementPostLabels[el]);

      if (el == elementsArrayLength - 1) {
        if (longDataLength > lcdCols * lcdRows) longDataLength = lcdCols * lcdRows + 3;
        offset = 0;
        for (byte rowNum = 0; rowNum < lcdRows; rowNum++) {
          lcd.setCursor(x, rowNum);
          charNum = rowNum * lcdCols + offset;
          while (charNum < ((rowNum + 1) *lcdCols) + offset) {
            if (charNum - offset == rowNum * lcdCols && charNum < longDataLength && text[charNum] == ' ') {
              charNum++;
              offset++;
              longDataLength--;
            }
            if (charNum - offset >= longDataLength) lcd.write(254);
            // if (charNum-offset >= longDataLength) lcd.print("x");
            else if (charNum - offset < longDataLength)lcd.print(text[charNum]);
            charNum++;
          }
        }
      }
#endif

#ifdef BIGTEXT
      char_x = x; char_y = y;
#ifdef BIGLABELS
      printBigCharacters(elementPreLabels[el], char_x, char_y);
#else
      for (byte i = 0; i < strlen (elementPreLabels[el]); i++) {
        lcd.setCursor(char_x + i, char_y);
        lcd.write(254);
      }
#endif
      lcd.setCursor(char_x, char_y + 1);
      lcd.print(elementPreLabels[el]);
      char_x = char_x + strlen (elementPreLabels[el]);
      printBigCharacters(elementValues[el], char_x, char_y);
#ifdef BIGLABELS
      printBigCharacters(elementPostLabels[el], char_x, char_y);
#else
      for (byte i = 0; i < strlen (elementPostLabels[el]); i++) {
        lcd.setCursor(char_x + i, char_y);
        lcd.write(254);
      }

      lcd.setCursor(char_x, char_y + 1);
      lcd.print(elementPostLabels[el]);
      char_x = char_x + strlen (elementPostLabels[el]);
#endif
      x = x + char_x;
#endif


#if !defined BIGTEXT && !defined LONGDATA
      lcd.print(elementPreLabels[el]); lcd.print(elementValues[el]); lcd.print(elementPostLabels[el]);
#endif

      if (el == elementsArrayLength - 1 || col == elementCols[cols] - 1) {


#if !defined GRID && !defined BIGTEXT
        while (dataLength < lcdCols) {
          lcd.write(254);   // comment this to debug
          //lcd.print("x"); // uncomment this debug
          dataLength++;
        }
#endif

#if !defined GRID && defined BIGTEXT
        while (char_x < lcdCols) {
          lcd.setCursor(char_x, char_y);
          lcd.write(254);
          lcd.setCursor(char_x, char_y + 1);
          lcd.write(254);
          char_x++;
        }
#endif
        x = 0;
        break;
      }
      el++;
    }
    cols++;
    y++;
#ifdef BIGTEXT
    y++;
#endif
  }
}



//---------------------- Large LCD Characters ------------------------//  Originally by Michael Pilcher, modified by Ben Lipsey
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
  B00000
};
byte RT[8] = //2
{
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte LL[8] = //3
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01111,
  B00111
};
byte LB[8] = //4
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
byte LR[8] = //5
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11100
};
byte UMB[8] =  //6
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};
byte LMB[8] =  //7
{
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
byte LT[8] =  //8
{
  B00111,
  B01111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

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


void printBigCharacters(char character[], byte x, byte y) {
  bool foundChar;
  byte charWidth = 0;
  for (byte i = 0; i < strlen(character); i++) {

    charWidth = 0;
    foundChar = false;

    if (character[i] == '0' || character[i] == 'O' || character[i] == 'o')
    {
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

    if (character[i] == '1')
    {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(255);
    }

    if (character[i] == '2')
    {
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

    if (character[i] == '3')
    {
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

    if (character[i] == '4')
    {
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

    if (character[i] == '5')
    {
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

    if (character[i] == '6')
    {
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

    if (character[i] == '7')
    {
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

    if (character[i] == '8')
    {
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

    if (character[i] == '9')
    {
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

    if (character[i] == 'A' || character[i] == 'a')
    {
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

    if (character[i] == 'B' || character[i] == 'b')
    {
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

    if (character[i] == 'C' || character[i] == 'c')
    {
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

    if (character[i] == 'D' || character[i] == 'd')
    {
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

    if (character[i] == 'E' || character[i] == 'e')
    {
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

    if (character[i] == 'F' || character[i] == 'f')
    {
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

    if (character[i] == 'G' || character[i] == 'g')
    {
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

    if (character[i] == 'H' || character[i] == 'h')
    {
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

    if (character[i] == 'I' || character[i] == 'i')
    {
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

    if (character[i] == 'J' || character[i] == 'j')
    {
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

    if (character[i] == 'K' || character[i] == 'k')
    {
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

    if (character[i] == 'L' || character[i] == 'l')
    {
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

    if (character[i] == 'M' || character[i] == 'm')
    {
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

    if (character[i] == 'N' || character[i] == 'n')
    {
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

    if (character[i] == 'P' || character[i] == 'p')
    {
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

    if (character[i] == 'Q' || character[i] == 'q')
    {
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

    if (character[i] == 'R' || character[i] == 'r')
    {
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

    if (character[i] == 'S' || character[i] == 's')
    {
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

    if (character[i] == 'T' || character[i] == 't')
    {
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

    if (character[i] == 'U' || character[i] == 'u')
    {
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

    if (character[i] == 'V' || character[i] == 'v')
    {
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

    if (character[i] == 'W' || character[i] == 'w')
    {
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

    if (character[i] == 'X' || character[i] == 'x')
    {
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

    if (character[i] == 'Y' || character[i] == 'y')
    {
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

    if (character[i] == 'Z' || character[i] == 'z')
    {
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

    if (character[i] == '?')
    {
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

    if (character[i] == '!')
    {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write(255);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
    }
    if (character[i] == ':')
    {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.print(".");
      lcd.setCursor(x, y + 1);
      lcd.print(".");
    }
    if (character[i] == ' ')
    {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(254);
    }

    if (character[i] == '.')
    {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write(254);
      lcd.setCursor(x, y + 1);
      lcd.write(4);
    }
    if (character[i] == '-')
    {
      foundChar = true;
      charWidth = 2;
      lcd.setCursor(x, y);
      lcd.write(4);
      lcd.write(4);
      lcd.setCursor(x, y + 1);
      lcd.write(254);
      lcd.write(254);
    }
    if (character[i] == '\"')
    {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write("\"");
      lcd.setCursor(x, y + 1);
      lcd.write(254);
    }
    if (character[i] == '\'')
    {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write("\'");
      lcd.setCursor(x, y + 1);
      lcd.write(254);
    }
    if (character[i] == '|')
    {
      foundChar = true;
      charWidth = 1;
      lcd.setCursor(x, y);
      lcd.write("|");
      lcd.setCursor(x, y + 1);
      lcd.write("|");
    }
    if (character[i] == '/')
    {
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

