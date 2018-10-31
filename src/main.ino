#include <WiFi.h>

WiFiClient client;
String ssid = WIFI_SSID;
String password = PASSWORD;
String server_host = HOST_IP_ADDRESS;

bool connect_wifi(const char *ssid, const char *password)
{
  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int local_count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (++local_count == 20)
    {
      return false;
    }
    Serial.print(".");
    delay(500);
  }
  return true;
}

int loop_count = 0;

void setup()
{
  Serial.begin(115200);
  delay(10);
  connect_wifi(ssid.c_str(), password.c_str());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);

  const uint16_t port = 5003;
  const char *host = server_host.c_str(); // ip or dns

  Serial.print("connecting to ");
  Serial.println(host);

  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }
}

void loop()
{
  client.print("Client Ready");
  client.print(++loop_count);

  //read back one line from server
  String line = client.readStringUntil('\r');
  while (line == "")
  {
    delay(1000);
    line = client.readStringUntil('\r');
  }
  Serial.println(line);
}