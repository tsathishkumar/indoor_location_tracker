#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEBeacon.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define TOPIC "location/node1"
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

WiFiClient client;
String ssid = WIFI_SSID;
String password = PASSWORD;
String mqtt_host = HOST_IP_ADDRESS;
int scanTime = 15; //In seconds
char minor[20], major[20];
BLEBeacon myBeacon;
BLEScan *pBLEScan;

PubSubClient mqtt_client(client);

void setup_mqtt()
{
  mqtt_client.setServer(mqtt_host.c_str(), 1883);
  int mqtt_count_local = 0;

  Serial.print("MQTT with ");
  Serial.println(mqtt_host);
  while (!mqtt_client.connected())
  {
    Serial.print(".");
    Serial.print(mqtt_client.state());
    mqtt_count_local++;
    if (mqtt_count_local == 15)
      ESP.restart();

    if (mqtt_client.connect("node1"))
    {
      Serial.println("connected to MQTT");
    }
    else
      delay(1000);
  }
}

void check_mqtt_connection()
{
  if (!mqtt_client.connected())
    setup_mqtt();
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{

  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    int minor, major, rssi;
    BLEAdvertisedDevice *advertisedDeviceTemp = &advertisedDevice;
    if (advertisedDeviceTemp->getManufacturerData().length() != 25)
    {
      return;
    }
    myBeacon.setData(advertisedDeviceTemp->getManufacturerData());
    // Add only the beacons that we are interested in
    minor = ENDIAN_CHANGE_U16(myBeacon.getMinor());
    major = ENDIAN_CHANGE_U16(myBeacon.getMajor());
    rssi = advertisedDeviceTemp->getRSSI();
    if (!(minor == 100 && major == 100))
    {
      print_beacon_details(minor, major, rssi);
      publish_asset_to_mqtt(minor, major, rssi);
    }
  }

  void print_beacon_details(int minor, int major, int rssi)
  {

    Serial.print(minor);
    Serial.print(",");
    Serial.print(major);
    Serial.print("=");
    Serial.print(rssi);
    Serial.println("");
  }

  void publish_asset_to_mqtt(int minor_int, int major_int, int rssi)
  {
    char minor[20], major[20];
    sprintf(minor, "%d", minor_int);
    sprintf(major, "%d", major_int);
    strcat(minor, major);
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["rssi"] = rssi;
    root["location_id"] = minor;
    char jsonChar[100];
    root.printTo((char *)jsonChar, root.measureLength() + 1);
    mqtt_client.publish(TOPIC, jsonChar);
  }
};

void setup_ble()
{
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

int scan_beacons()
{
  BLEDevice::init("");
  BLEScanResults foundDevices = pBLEScan->start(scanTime);
  Serial.print(" - Scan done!!");
  int device_count = foundDevices.getCount();
  Serial.print(" Devices found: ");
  Serial.println(device_count);
  return device_count;
}

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

void setup()
{
  Serial.begin(115200);
  delay(10);
  connect_wifi(ssid.c_str(), password.c_str());
  setup_mqtt();
  setup_ble();
}

void loop()
{
  scan_beacons();
}