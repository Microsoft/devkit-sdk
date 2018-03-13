#include "WebSocketClient.h"

#define MAX_TRY_WRITE 30
#define MAX_TRY_READ 10

// Currently we used a pre-calculated pair of based-64 strings for WebSocket opening handshake.
// TODO: used a random generated key in client side and calculate the valid server response string
// according to the WebSockeet protocol defenition (see https://tools.ietf.org/html/rfc6455#section-1.3)
const char WS_HANDSHAKE_CLIENT_KEY[] = "L159VM0TWUzyDxwJEIEzjw==";
const char WS_HANDSHAKE_SERVER_ACCEPT[] = "DdLWT/1JcX+nQFHebYP+rqEx5xI=";

WebSocketClient::WebSocketClient(char *url)
{
    _tcpSocket = NULL;
    _parsedUrl = NULL;
    _parsedUrl = new ParsedUrl(url);

    if (!_parsedUrl->schema())
    {
        if (strcmp(_parsedUrl->schema(), "ws") == 0)
        {
            _port = 80;
        }
        else
        {
            // TODO: support secure WebSocket (port 443)
            _port = 443;
        }
    }
}

WebSocketClient::~WebSocketClient()
{
    if (_tcpSocket != NULL)
    {
        _tcpSocket->close();
        delete _tcpSocket;
        _tcpSocket = NULL;
    }

    if (_parsedUrl != NULL)
    {
        delete _parsedUrl;
        _parsedUrl = NULL;
    }
}

bool WebSocketClient::connect()
{
    if (_tcpSocket != NULL)
    {
        // Already connected.
        return true;
    }
    else
    {
        _tcpSocket = new TCPSocket();
        if (_tcpSocket == NULL)
        {
            return false;
        }

        if (_tcpSocket->open(WiFiInterface()) != 0 || _tcpSocket->connect(_parsedUrl->host(), _parsedUrl->port()) != 0)
        {
            ERROR_FORMAT("Unable to connect to %s on port %d\r\n", _parsedUrl->host(), _parsedUrl->port());

            delete _tcpSocket;
            _tcpSocket = NULL;
            return false;
        }

        _tcpSocket->set_timeout(1000);
    }

    return doHandshake();
}

bool WebSocketClient::doHandshake()
{
    char strBuffer[200];

    // Send handshake message to upgrade http to the ws protocol
    if (strlen(_parsedUrl->query()) > 0)
    {
        sprintf(strBuffer, "GET %s?%s HTTP/1.1\r\n", _parsedUrl->path(), _parsedUrl->query());
    }
    else
    {
        sprintf(strBuffer, "GET %s HTTP/1.1\r\n", _parsedUrl->path());
    }
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Host: %s:%d\r\n", _parsedUrl->host(), _parsedUrl->port());
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Upgrade: WebSocket\r\n");
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Connection: Upgrade\r\n");
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Sec-WebSocket-Key: %s\r\n", WS_HANDSHAKE_CLIENT_KEY);
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Sec-WebSocket-Version: 13\r\n\r\n");
    int ret = write(strBuffer, strlen(strBuffer));
    if (ret != strlen(strBuffer))
    {
        close();
        ERROR("Could not send request.");
        return false;
    }

    // Receive handshake response from WebSocket server
    Timer timer;
    timer.start();

    do
    {
        ret = read(strBuffer, 200, 100);
        if (ret < 0)
        {
            ERROR("Unable to get handshake response from server.");
            return false;
        }
        else
        {
            strBuffer[ret] = '\0';

            // Server accepted the client handshake
            if (strstr(strBuffer, WS_HANDSHAKE_SERVER_ACCEPT) != NULL)
            {
                return true;
            }
        }
    } while (ret > 0 && timer.read_ms() < TIMEOUT_IN_MS);

    return false;
}

bool WebSocketClient::connected()
{
    return (_tcpSocket == NULL) ? false : true;
}

int WebSocketClient::sendLength(uint32_t len, char *msg)
{
    if (len < 126)
    {
        msg[0] = len | (1 << 7);
        return 1;
    }
    else if (len < 65535)
    {
        msg[0] = 126 | (1 << 7);
        msg[1] = (len >> 8) & 0xff;
        msg[2] = len & 0xff;
        return 3;
    }
    else
    {
        msg[0] = 127 | (1 << 7);
        for (int i = 0; i < 8; i++)
        {
            msg[i + 1] = (len >> i * 8) & 0xff;
        }
        return 9;
    }
}

int WebSocketClient::readChar(char *pC, bool block)
{
    int ret = read(pC, 1, 1);
    return ret;
}

int WebSocketClient::sendMask(char *msg)
{
    for (int i = 0; i < 4; i++)
    {
        msg[i] = 0x00;
    }
    return 4;
}

int WebSocketClient::send(char *str, int size)
{
    if (_tcpSocket == NULL)
    {
        ERROR("unable to send data when WebSocket is disconnected.");
        return -1;
    }

    if (str == NULL || size <= 0)
    {
        ERROR("Input data string is NULL or size is invalid.");
        return -1;
    }

    char msg[15];
    int idx = 1;
    msg[0] = 0x81;
    if (size == 0)
    {
        size = strlen(str);
    }

    idx += sendLength(size, msg + idx);
    idx += sendMask(msg + idx);
    int res = write(msg, idx);
    if (res != idx)
    {
        ERROR("Send websocket frame header failed.");
        return -1;
    }

    res = write(str, size);
    return res == -1 ? -1 : res + idx;
}

WebSocketReceiveResult *WebSocketClient::receive(char *message)
{
    if (_tcpSocket == NULL)
    {
        ERROR("Unable to receive data when WebSocket is disconnected.");
        return NULL;
    }

    int i = 0;
    uint32_t payloadLength;
    char recvByte = 0;
    char opcode = 0;
    char c;
    char mask[4] = {0, 0, 0, 0};
    bool isMasked = false;
    bool isFinal = false;
    Timer timer;

    // Read opcode
    timer.start();
    while (true)
    {
        if (timer.read() > 10)
        {
            ERROR("WebSocket timeout\r\n");
            return NULL;
        }

        int res = _tcpSocket->recv(&recvByte, 1);

        if (res == 1)
        {
            _tcpSocket->set_timeout(2000);
            opcode = recvByte & 0x7F;

            // opcode for data frames
            if (opcode == WS_OPCODE_CONT || opcode == WS_OPCODE_TEXT || opcode == WS_OPCODE_BINARY)
            {
                if (opcode == WS_OPCODE_TEXT)
                {
                    messageType = WS_Message_Text;
                }
                else if (opcode == WS_OPCODE_BINARY)
                {
                    messageType = WS_Message_Binary;
                }

                isFinal = ((recvByte & 0x80) == 0x80);
                break;
            }
            // opcode for connection close
            else if (opcode == WS_OPCODE_CLOSE)
            {
                close();
            }
        }
        else if (res < 0 && res != NSAPI_ERROR_WOULD_BLOCK)
        {
            ERROR_FORMAT("Socket receive failed, res: %d, opcode: %x\r\n", res, opcode);
            return NULL;
        }
    }

    // Parse payload length
    readChar(&c);
    payloadLength = c & 0x7f;
    isMasked = c & 0x80;
    if (payloadLength == 126)
    {
        readChar(&c);
        payloadLength = c << 8;
        readChar(&c);
        payloadLength += c;
    }
    else if (payloadLength == 127)
    {
        payloadLength = 0;
        for (int i = 0; i < 8; i++)
        {
            readChar(&c);
            payloadLength += (c << (7 - i) * 8);
        }
    }

    if (payloadLength == 0)
    {
        return NULL;
    }

    if (isMasked)
    {
        for (i = 0; i < 4; i++)
        {
            readChar(&c);
            mask[i] = c;
        }
    }

    int nb = read(message, payloadLength, payloadLength);
    if (nb != payloadLength)
        return NULL;

    for (i = 0; i < payloadLength; i++)
    {
        message[i] = message[i] ^ mask[i % 4];
    }

    message[payloadLength] = '\0';

    WebSocketReceiveResult *receiveResult = new WebSocketReceiveResult();
    receiveResult->isEndOfMessage = isFinal;
    receiveResult->length = payloadLength;
    receiveResult->messageType = messageType;
    receiveResult->error = NSAPI_ERROR_OK;

    return receiveResult;
}

bool WebSocketClient::close()
{
    // Send a close frame (0x8700) to server to tell it client is closing from here.
    write((uint8_t)0x8700, 8);

    if (_tcpSocket == NULL)
    {
        printf("WebSocket is already disconnected.");
        return true;
    }

    int ret = _tcpSocket->close();
    if (ret < 0)
    {
        ERROR("Could not disconnect to WebSocket server.");
        return false;
    }

    delete _tcpSocket;
    _tcpSocket = NULL;
    return true;
}

const char *WebSocketClient::getPath()
{
    return _parsedUrl->path();
}

int WebSocketClient::write(char *str, int len)
{
    int res = 0, idx = 0;

    for (int j = 0; j < MAX_TRY_WRITE; j++)
    {
        if ((res = _tcpSocket->send(str + idx, len - idx)) < 0)
        {
            continue;
        }

        idx += res;

        if (idx == len)
        {
            return len;
        }

        wait_ms(40);
    }

    return (idx == 0) ? -1 : idx;
}

int WebSocketClient::read(char *str, int len, int min_len)
{
    int res = 0, idx = 0;

    for (int j = 0; j < MAX_TRY_WRITE; j++)
    {

        if ((res = _tcpSocket->recv(str + idx, len - idx)) < 0)
            continue;

        idx += res;

        if (idx == len || (min_len != -1 && idx > min_len))
            return idx;
    }

    return (idx == 0) ? -1 : idx;
}
