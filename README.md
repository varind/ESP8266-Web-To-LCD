# ESP8266-Web-To-LCD
Display select data from XML/RSS/HTML on an LCD using ESP8266 and Arduino IDE

Check out the project page: http://www.variableindustries.com/web-to-lcd-2/



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
