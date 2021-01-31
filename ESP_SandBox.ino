#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266WebServer.h>
#include <LittleFS.h>   // Include the SPIFFS library
#include <WebSocketsServer.h>
#include <FTPServer.h>


ESP8266WebServer Server(80);
ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
WebSocketsServer webSocket(81);    // create a websocket server on port 81
FTPServer FTP(LittleFS);

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

bool isConnected = false;

uint64_t previousTime;
uint64_t dt;

uint32_t date = 0;

void setup()
{
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(D2, OUTPUT);

  pinMode(A0, INPUT);

  wifiMulti.addAP("WiYourte", "yourte");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("TooT", "1234567890");
  wifiMulti.addAP("ALTICE-6BB1", "8FQND3DR7N2");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (!MDNS.begin("yourte"))              // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");

  Serial.println("mDNS responder started");

  LittleFS.begin();                           // Start the SPI Flash Files System and FTP server
  FTP.begin("TooT", "1234567890");
  FTP.setTimeout(30000);

  Server.onNotFound([]()
		  {                              // If the client requests any URI
			if (!handleFileRead(Server.uri()))                  // send it if it exists
			  Server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
		  });

  Server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");

  previousTime = millis();
}

void loop(void)
{
  Server.handleClient();
  webSocket.loop();
  FTP.handleFTP();

  uint64_t currentTime = millis();
  dt += currentTime - previousTime;

  if(dt > 10000)
  {
	  File data;

	  if(date == 0)
	  {
		  data = LittleFS.open("/WiYourte.csv", "w");
		  data.println("Date,Value");
	  }

	  else
		  data = LittleFS.open("/WiYourte.csv", "a");

	  data.print(date);
	  data.print(",");

	  uint16_t value = analogRead(A0);

	  Serial.println(value);
	  data.println(value);

	  data.close();

	  date++;
	  dt = 0;
  }

  previousTime = currentTime;

  if(isConnected)
	  digitalWrite(2, LOW);
  else
	  digitalWrite(2, HIGH);
}

String getContentType(String filename)
{
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);

  if (path.endsWith("/"))
	  path += "index.html";

  String contentType = getContentType(path);

  if (LittleFS.exists(path))
  {
    File file = LittleFS.open(path, "r");
    Server.streamFile(file, contentType);
    file.close();
    return true;
  }

  Serial.println("\tFile Not Found");
  return false;
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght)
{
	switch (type)
	{
		case WStype_DISCONNECTED:
		  Serial.printf("[%u] Disconnected!\n", num);
		  isConnected = false;
		  break;

		case WStype_CONNECTED:
			{
				IPAddress ip = webSocket.remoteIP(num);
				Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				isConnected = true;                  // Turn rainbow off when a new connection is established
			}
		  break;

	    case WStype_TEXT:                     // if new text data is received
	    	char* s = (char*)(payload);
			uint16_t r = atoi(s);
			analogWrite(D2, r);                         // write it to the LED output pins
		break;
	}
}
