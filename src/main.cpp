/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <WiFi.h>
#include<time.h>

#include <SPI.h>
#include <LoRa.h>

#define ss 18 
#define rst 14
#define dio0 2
int c=0;

const char* ssid     = "iPhone von Hauke";
const char* password = "keinpasswort";
const char* iphost="raincloud.bplaced.net";

const char* host =  "api.openweathermap.org";
const char* privateKey = "3dd27852d3ad61da5c6c32f688898aa4";
const char* streamId   = "....................";

String lat_ ="N";
double lat  =48.77;
String lng_ ="E";
double lng  =9.18;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup()
{

    inputString = "";
    
    
    Serial.begin(9600);
    //Serial1.begin(9600);
     while (!Serial);
//replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  //pinMode(ledpin,OUTPUT);
   LoRa.setPins(ss, rst, dio0);
     
    Serial.println("LoRa Receiver");

     while (!LoRa.begin(868E6)) {
    
    Serial.print(".");
    if(c++>=50){c=0;Serial.println(".");}
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

    // We start by connecting to a WiFi network    

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

   

    int timezone = 2;
int dst = 0;
   configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  Serial.println("");
}

int value = 0;





String postData(const char* thishost,String thisurl,String PostData){
  //ledON();
  Serial.print("connecting to "); 
  Serial.println(thishost);  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(thishost, httpPort)) {
    Serial.println("connection failed");
    return "connection failed";
  }
  Serial.print("Requesting URL: ");
  Serial.println(thisurl);
  // This will send the request to the server
  client.println("POST "+thisurl+" HTTP/1.1");
  client.println("Host: "+String(thishost));               //was macht das
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/x-www-form-urlencoded");  //was macht das
  client.print("Content-Length: ");
  client.println(PostData.length());
  client.println();
  client.println(PostData);
                 
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "timeout";
    }
  }
  //Serial.println("das habe ich gefunden :");
  String line="";
  while(client.available()){
    line += client.readStringUntil('\r');
    
   
  }
  //Serial.print(line);
  //Serial.println("Was macht man jetzt damit");

  return line;
  
  //ledOFF();
}

void loop()
{
// try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LoRaData="";
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
       LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    String url="/sentLora.php";
    Serial.println(postData(iphost,url,LoRaData));
   
  }
    
}
