/*
 * 31 mar 2015
 * This sketch display UDP packets coming from an UDP client.
 * On a Mac the NC command can be used to send UDP. (nc -u 192.168.1.101 2390).
 *
 * Configuration : Enter the ssid and password of your Wifi AP. Enter the port number your server is listening on.
 *
  13 apr 2015. Partly working. Packets are received and displayed, but the confirmation packet is not received by the client.
  Tested with Mac as cleint using netcat (nc -u 192.168.1.101 2390)

 */

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

int status = WL_IDLE_STATUS;
const char* ssid = "Freebox";  //  your network SSID (name)
const char* pass = "anewfuckedkey";       // your network password

unsigned int localPort = 3671;      // local port to listen for UDP packets

byte packetBuffer[512]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

// Multicast declarations
IPAddress ipMulti(224, 0, 23, 12);
unsigned int portMulti = 3671;      // local port to listen on
byte ledSwitch=0;
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
  // set pin 16 (conneted to blue LED) to output
  pinMode(16, OUTPUT);          
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // setting up Station AP
  WiFi.begin(ssid, pass);
  // Wait for connect to AP
  Serial.print("[Connecting]");
  Serial.print(ssid);
  int tries=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tries++;
    if (tries > 30) {
      break;
    }
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
    if (ledValue>=0 && ledValue<=255) ledValue=ledValue+dimmingDirection;
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
