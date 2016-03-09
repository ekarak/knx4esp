/*
 this code is provided under GPLV2, please don't sue me,
 multicast sample from http://www.esp8266.com/viewtopic.php?f=29&t=2464 thanks
 thanks to the knxd team for the gateway and to buswar.de for the usb dongle: http://busware.de/tiki-index.php?page=TUL
 */

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

int status = WL_IDLE_STATUS;
const char* ssid = "Freebox";  //  your network SSID (name)
const char* pass = "anewfuckedkey";       // your network password

unsigned int localPort = 3671;      // local port to listen for UDP packets

byte packetBuffer[512]; //buffer to hold incoming and outgoing packets

WiFiUDP Udp;  // A UDP instance to let us send and receive packets over UDP

// Multicast declarations
IPAddress ipMulti(224, 0, 23, 12);
unsigned int portMulti = 3671;      // local port to listen on
byte ledSwitch=0;

// led status declaration
int ledValue=255;
int dimmingDirection=0;
int loopNumber=0;


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}


void setup() {
  pinMode(16, OUTPUT);      // set pin 16 (conneted to blue LED) to output
  Serial.begin(115200);     // Open serial communications and wait for port to open:
  WiFi.begin(ssid, pass);   // setting up Station AP
  
  // Wait for connect to AP
  Serial.print("[Connecting]");
  Serial.print(ssid);
  int tries=0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();


  printWifiStatus();

  Serial.println("Connected to wifi");
  Serial.print("Udp Multicast server started at : ");
  Serial.print(ipMulti);
  Serial.print(":");
  Serial.println(portMulti);
  Udp.beginMulticast(WiFi.localIP(),  ipMulti, portMulti);
}

void loop()
{
  loopNumber++;
  if (loopNumber>500) { // if dimming, do it once each 500 loop to be humanly acceptable
    loopNumber=0;       //reset loop number
    if (ledValue>=0 && ledValue<=255) ledValue=ledValue+dimmingDirection; // continue dimming if needed
    if (ledValue<0) ledValue=0;     //should never happen but better be safe than sorry
    if (ledValue>255) ledValue=255; //should never happen but better be safe than sorry
  }
  // output the ledvalue, on this board the led is connected between +5V and GPIO16 so we do some math to convert ledValue to something correct
  analogWrite(16,1024-(ledValue*4)*ledSwitch);
 
  int noBytes = Udp.parsePacket();
 
  if ( noBytes ) {
    /*Serial.print(millis() / 1000);
    Serial.print(":Packet of ");
    Serial.print(noBytes);
    Serial.print(" received from ");
    Serial.print(Udp.remoteIP());
    Serial.print(":");
    Serial.println(Udp.remotePort());*/
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,noBytes); // read the packet into the buffer

    if ( packetBuffer[2]==0x05 && packetBuffer[3]==0x30  // routing indication
    && (packetBuffer[9]&0x80)==0x80                 // destination de type group address
    && (((packetBuffer[15]&0x03)<<2)+((packetBuffer[16]&0xC0)>>6))==02  //groupwrite
    ) {
      if ( packetBuffer[12]*256+packetBuffer[13]==2309 )  // adresse de destination switch 1/1/5
      {
        Serial.print("switching value :");
        Serial.println(packetBuffer[16]&0x3F,HEX);
        ledSwitch=packetBuffer[16]&0x3F&0x01;
      }

      if ( packetBuffer[12]*256+packetBuffer[13]==2565)  // adresse de destination dimming 1/2/5
      {
        Serial.print("dimming value ?");
        Serial.println(packetBuffer[16]&0x3F,HEX);
 
        if ( packetBuffer[16]&0x8)
          dimmingDirection=1;
        else
          dimmingDirection=-1;
 
        if ( (packetBuffer[16]&0x3)==0 )
          dimmingDirection=0;
      }
    }
    Serial.print("ledValue :");
    Serial.println(ledValue);
 
 /*   for (int i=1;i<=noBytes;i++){
      Serial.print(packetBuffer[i-1],HEX);
      if (i % 32 == 0){
        Serial.println();
      }
      else Serial.print(' ');
    } // end for
    Serial.println();
    */
   // send a reply, to the IP address and port that sent us the packet we received
  //  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  //  Udp.write("UDP packet received");
  //  Udp.endPacket();   
  } // end if
}
