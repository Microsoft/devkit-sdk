#include "AZ3166WiFi.h"
#include "WebSocketClient.h"

static bool hasWifi;
static bool isWsConnected;
static char *webSocketServerUrl = "ws://devkitwebsocket.azurewebsites.net/test?nickName=test2";
static WebSocketClient *wsClient;
char wsBuffer[1024];
char wifiBuff[128];
int msgCount;

void InitWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    snprintf(wifiBuff, 128, "WiFi Connected\r\n%s\r\n%s\r\n \r\n", WiFi.SSID(), ip.get_address());
    Screen.print(wifiBuff);

    hasWifi = true;
  }
  else
  {
    snprintf(wifiBuff, 128, "No WiFi\r\nEnter AP Mode\r\nto config\r\n                 \r\n");
    Screen.print(wifiBuff);
  }
}

bool connectWebSocket()
{
  Screen.clean();
  Screen.print(0, "Connect to WS...");

  if (wsClient == NULL)
  {
    wsClient = new WebSocketClient(webSocketServerUrl);
  }

  isWsConnected = wsClient->connect();
  if (isWsConnected)
  {
    Screen.print(1, "connect WS successfully.");
    Serial.println("WebSocket connect successfully.");
  }
  else
  {
    Screen.print(1, "Connect WS failed.");
    Serial.print("WebSocket connection failed, isWsConnected: ");
    Serial.println(isWsConnected);
  }

  return isWsConnected;
}

void setup()
{
  hasWifi = false;
  isWsConnected = false;
  msgCount = 0;

  InitWiFi();
  if (hasWifi)
  {
    connectWebSocket();
  }

  pinMode(USER_BUTTON_A, INPUT);
}

void loop()
{
  if (hasWifi)
  {
    if (!isWsConnected)
    {
      Screen.print(0, "DevKit WebSocket");
      Screen.print(1, "Press button A to  connect WS.", true);

      while (digitalRead(USER_BUTTON_A) != LOW)
      {
        delay(10);
      }

      connectWebSocket();
    }

    if (isWsConnected)
    {
      int len = 0;
      char msgBuffer[128];
      unsigned char opcode = 0;

      for (int i = 0; i < 3; i++)
      {
        // Send message to WebSocket server
        sprintf(msgBuffer, "Hello WebSocket %d", msgCount);
        int res = wsClient->send(msgBuffer);
        if (res > 0)
        {
          Screen.clean();
          Screen.print(0, "WS send:");
          Screen.print(1, msgBuffer, true);
          Serial.printf("Send message %d successfully.\r\n", msgCount);
          msgCount++;
        }

        // Receive message from WebSocket Server
        bool isEndOfMessage = false;
        WebSocketReceiveResult *recvResult = NULL;
        while (!isEndOfMessage)
        {
          recvResult = wsClient->receive(wsBuffer);

          if (!recvResult && recvResult->length > 0)
          {
            len = recvResult->length;
            isEndOfMessage = recvResult->isEndOfMessage;

            Screen.clean();
            Screen.print(0, "WS receive:");
            Screen.print(1, wsBuffer, true);
            Serial.println(wsBuffer);

            memset(wsBuffer, 0, sizeof(wsBuffer));
            delay(100);
          }
        }
      }

      // Close WebSocket after sending/receiving messages
      if (wsClient->close())
      {
        isWsConnected = false;
        Screen.print(1, "Close WS successfully.", true);
        Serial.println("Close WS successfully.");

        free(wsClient);
        wsClient = NULL;
      }
      else
      {
        Screen.print(1, "Close WS failed.", true);
        Serial.println("Close WS failed.");
      }
    }
  }

  delay(1000);
}
