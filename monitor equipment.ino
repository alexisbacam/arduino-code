#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include "DHT.h"

#include <WiFiNINA.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

WiFiClient client;
// ifttt parameters
#define HOSTIFTTT "maker.ifttt.com"
#define EVENTO "your event-name" //change with your event name
#define IFTTTKEY "your key" // change with your key

//dth 
#define DHTPIN 2
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(-1);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
int sensorPin = A2;

/* variables*/
double tm=0;
int tmt=0;
float h=0;
double lb=0;
int lbt=0;

void setup() {
  //init pin led
  pinMode(6, OUTPUT);
  //init pin battery level
  pinMode(sensorPin, INPUT);
  //init oled
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.display();
  delay(2000);
  display.clearDisplay();
  //init sensor mlx90614 - recomended put sensor on 3cm distance max.
  mlx.begin();
  dht.begin();
  
  //wifi
   //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

void loop() {
    
      // To blink the LED, first we'll turn it on...
  digitalWrite(6, HIGH);

  // Then we'll turn it off...
  
//Read temperature motor
    tmt = (mlx.readObjectTempC())*100;
    tm =tmt*1.0/100;

//read humidity, some cases, when try to read humidity, sensor read null; for this case, read again
    double hum =dht.readHumidity();
    while(isnan(hum)) {
		delay(1000);
		hum =dht.readHumidity();
	  }
	  h = hum;
// read battery level
 //   lbt = (((float)3.3*analogRead(sensorPin)/1024)/0.8)*100;
 //   lb = lbt*1.0/100;
 lb=(16.5*analogRead(sensorPin)/1024);
    
//show data on display    
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("EQUIPMENT MONITOR");
  display.print("Motor: ");display.print(tm);display.println(" C");
  display.print("Humidity: ");display.print(h);display.println(" %");
  display.print("Battery: ");display.print(lb);display.println(" v");
  display.display();
  display.clearDisplay();
  
  Serial.println(tm);
  Serial.println(lb);
  Serial.println(h);
 
  enviar_tweet(tm, lb, h);
  digitalWrite(6, LOW);
  delay(5000);
}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void enviar_tweet(double valor1, double valor2, float valor3)
{
  // Cerramos cualquier conexión anterior
  if (client.connected())
  {
    client.stop();
  }
 
  // Esperamos hasta que se hayan enviado todos los datos
  client.flush();
 
  // Hacemos la petición mediante SSL
  if (client.connectSSL(HOSTIFTTT, 443)) {
    // Construimos la petición HTTP
    String toSend = "GET /trigger/";
    toSend += EVENTO;
    toSend += "/with/key/";
    toSend += IFTTTKEY;
    toSend += "?value1=";
    toSend += valor1;
    toSend += "&value2=";
    toSend += valor2;
    toSend += "&value3=";
    toSend += valor3;
    toSend += " HTTP/1.1\r\n";
    toSend += "Host: ";
    toSend += HOSTIFTTT;
    toSend += "\r\n";
    toSend += "Connection: close\r\n\r\n";
    client.print(toSend);
  }
 
  // Esperamos hasta que se hayan enviado todos los datos
  client.flush();
  // Desconectamos del cliente
  client.stop();
}
