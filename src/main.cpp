#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

#define myID 3

#define ss 18
#define rst 14
#define dio0 2
const char *ssid = "Freifunk";
const char *iphost = "raincloud.bplaced.net";
/*
const char *host = "api.openweathermap.org";
const char *privateKey = "3dd27852d3ad61da5c6c32f688898aa4";
const char *streamId = "....................";
*/
TinyGPSPlus gps;
double lat = 48.770814;
double lng = 9.175058;
int counter = 0;

String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

void setup()
{
  inputString = "";
  Serial.begin(9600);
  //Serial1.begin(9600);
  while (!Serial)
    ;
  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  //pinMode(ledpin,OUTPUT);
  LoRa.setPins(ss, rst, dio0);
  Serial.println("LoRa Receiver");
  int c = 0;
  while (!LoRa.begin(868E6))
  {

    Serial.print(".");
    if (c++ >= 50)
    {
      c = 0;
      Serial.println(".");
    }
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // WiFi.begin(ssid, password);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
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
  c = 0;
  while (!time(nullptr))
  {
    Serial.print(".");
    if (c++ >= 50)
    {
      c = 0;
      Serial.println(".");
    }
    delay(5000);
  }
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  Serial.println("");
}

int value = 0;

String postData(const char *thishost, String thisurl, String PostData)
{
  //ledON();
  Serial.print("connecting to ");
  Serial.println(thishost);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(thishost, httpPort))
  {
    Serial.println("connection failed");
    return "connection failed";
  }
  Serial.print("Requesting URL: ");
  Serial.println(thisurl);
  // This will send the request to the server
  client.println("POST " + thisurl + " HTTP/1.1");
  client.println("Host: " + String(thishost)); //was macht das
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/x-www-form-urlencoded"); //was macht das
  client.print("Content-Length: ");
  client.println(PostData.length());
  client.println();
  client.println(PostData);

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "timeout";
    }
  }
  //Serial.println("das habe ich gefunden :");
  String line = "";
  while (client.available())
  {
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
  if (packetSize)
  {
    String LoRaData = "";
    int RSSI = LoRa.packetRssi();
    // received a packet
    Serial.print("Received packet '");
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(RSSI);

    // read packet
    while (LoRa.available())
    {

      LoRaData = LoRa.readString();

      Serial.print(LoRaData);
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    int rssi_rx = LoRa.packetRssi();
    Serial.println(rssi_rx);

    String url = "/sentLora.php";
    String json_Msg = "{";
    json_Msg += ("\"MsgNr\":");
    json_Msg += (counter);
    json_Msg += (", \"RxID\":");
    json_Msg += (myID);
    json_Msg += (", \"RSSI\":");
    json_Msg += (rssi_rx);
    if (gps.date.isValid())
    {
      json_Msg += (", \"Date\":\"");
      if (gps.date.day() < 10)
        json_Msg += ("0");
      json_Msg += (gps.date.day());
      json_Msg += (".");
      if (gps.date.month() < 10)
        json_Msg += ("0");
      json_Msg += (gps.date.month());
      json_Msg += (".");
      json_Msg += (gps.date.year());
      json_Msg += ("\"");
    }
    if (gps.time.isValid())
    {
      json_Msg += (", \"Time\":\"");
      if (gps.time.hour() < 10)
        json_Msg += ("0");
      json_Msg += (gps.time.hour());
      json_Msg += (":");
      if (gps.time.minute() < 10)
        json_Msg += ("0");
      json_Msg += (gps.time.minute());
      json_Msg += (":");
      if (gps.time.second() < 10)
        json_Msg += ("0");
      json_Msg += (gps.time.second());
      json_Msg += ("\"");
    }
    if (gps.location.isValid())
    {
      lat = gps.location.lat();
      lng = gps.location.lng();
    }
    json_Msg += (", \"Location\":");
    json_Msg += ("{\"lat\":");
    json_Msg += String(lat, 6);
    json_Msg += (",");
    json_Msg += ("\"lng\":");
    json_Msg += String(lng, 6);
    json_Msg += ("}");

    json_Msg += (", \"Payload\":");
    json_Msg += (LoRaData);
    //LoRa.print("\"");
    json_Msg += ("}");
    ++counter;
    Serial.println(postData(iphost, url, json_Msg));
  }
}
