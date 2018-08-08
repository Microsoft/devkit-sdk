#include <AZ3166WiFi.h>
#include "MQTTClient.h"
#include "MQTTNetwork.h"
#include "Telemetry.h"

int status = WL_IDLE_STATUS;
int arrivedcount = 0;
bool hasWifi = false;

const char* mqttServer = "iot.eclipse.org";   //"m2m.eclipse.org";
int port = 1883;

void initWifi()
{
  Screen.print("IoT DevKit\r\n \r\nConnecting...\r\n");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;

    char msgInfo[60];
    sprintf(msgInfo, "Message arrived: qos %d, retained %d, dup %d, packetid %d", message.qos, message.retained, message.dup, message.id);
    Serial.println(msgInfo);

    sprintf(msgInfo, "Payload: %s", (char*)message.payload);
    Serial.println(msgInfo);
    ++arrivedcount;

    sprintf(msgInfo, "Message %d\r\n", arrivedcount);
    Screen.print(3, msgInfo);
    
    digitalWrite(LED_USER, HIGH);
    delay(500);
    digitalWrite(LED_USER, LOW);
    delay(500);
}

int runMqttExample() {
  char* topic = "mqtt-sample";
  MQTTNetwork mqttNetwork;
  MQTT::Client<MQTTNetwork, Countdown> client = MQTT::Client<MQTTNetwork, Countdown>(mqttNetwork);
  arrivedcount = 0;
  
  char msgBuf[100];
  sprintf(msgBuf, "Connecting to MQTT server %s:%d", mqttServer, port);
  Serial.println(msgBuf);

  int rc = mqttNetwork.connect(mqttServer, port);
  if (rc != 0) {
    Serial.println("Connected to MQTT server failed");
    Screen.print(2, "Connect to Server failed", true);
    return -1;
  } else {
    Serial.println("Connected to MQTT server successfully");
  }

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = "mqtt-sample";
  data.username.cstring = "testuser";
  data.password.cstring = "testpassword";
  
  if ((rc = client.connect(data)) != 0) {
      Serial.println("MQTT client connect to server failed");
      Screen.print(2, "Connect to Server failed", true);
      return -1;
  }
  
  if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0) {
      Serial.println("MQTT client subscribe from server failed");
      Screen.print(2, "Subscribe failed", true);
      return -1;
  }
  
  MQTT::Message message;

  // QoS 0
  char buf[100];
  sprintf(buf, "QoS 0 message from AZ3166!");
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;
  message.payload = (void*)buf;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  while (arrivedcount < 1) {
      client.yield(100);
  }
 
  // QoS 1
  sprintf(buf, "QoS 1 message from AZ3166!");
  message.qos = MQTT::QOS1;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  
  while (arrivedcount < 2) {
      client.yield(100);
  }
  
  if ((rc = client.unsubscribe(topic)) != 0) {
      Serial.println("MQTT client unsubscribe from server failed");
      Screen.print(2, "Unsubscribe failed", true);
      return -1;
  }
  
  if ((rc = client.disconnect()) != 0) {
      Serial.println("MQTT client disconnect from server failed");
      Screen.print(2, "Disconnect failed", true);
      return -1;
  }
  
  mqttNetwork.disconnect();
  Serial.print("Finish message count: ");
  Serial.println(arrivedcount);
  Screen.print(3, "Finish count...");
  return 0;
}

void setup() {
  //Initialize serial and Wi-Fi:
  Serial.begin(115200);
  initWifi();
  pinMode(LED_USER, OUTPUT);
  if(hasWifi)
  {
    // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data_async("", "MQTTClientSetup", "");
  }
}

void loop() {
  Serial.println("\r\n>>>Enter Loop");

  if (hasWifi) {
    runMqttExample();
  }

  delay(5000);
}
