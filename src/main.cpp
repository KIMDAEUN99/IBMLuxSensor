#include <Arduino.h>
#include <IBMIOTF8266.h>

// USER CODE EXAMPLE : Publish Interval. The periodic update is normally recommended.
// And this can be a good example for the user code addition
String user_html = ""
                   "<p><input type='text' name='meta.pubInterval' placeholder='Publish Interval'>";
// USER CODE EXAMPLE : command handling

char *ssid_pfix = (char *)"IOTLux";
unsigned long lastPublishMillis = -pubInterval;
const int analogPin = 0;

void publishData()
{
    char mBuf[50];
    int light = 0;

    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    //조도 센서로부터 받은 데이터
    light = analogRead(analogPin);
    sprintf(mBuf, "%d", light);
    data["lux"] = mBuf;

    serializeJson(root, msgBuffer);
    client.publish(publishTopic, msgBuffer);
}

void handleUserCommand(JsonDocument *root)
{
    JsonObject d = (*root)["d"];
}

void message(char *topic, byte *payload, unsigned int payloadLength)
{
    byte2buff(msgBuffer, payload, payloadLength);
    StaticJsonDocument<512> root;
    DeserializationError error = deserializeJson(root, String(msgBuffer));

    if (error)
    {
        Serial.println("handleCommand: payload parse FAILED");
        return;
    }

    handleIOTCommand(topic, &root);
    if (!strcmp(updateTopic, topic))
    {
        // USER CODE EXAMPLE : meta data update
        // If any meta data updated on the Internet, it can be stored to local variable to use for the logic
        pubInterval = cfg["meta"]["pubInterval"];
        // USER CODE EXAMPLE
    }
    else if (!strncmp(commandTopic, topic, 10))
    { // strcmp return 0 if both string matches
        handleUserCommand(&root);
    }
}

void setup()
{
    Serial.begin(115200);

    initDevice();
    // If not configured it'll be configured and rebooted in the initDevice(),
    // If configured, initDevice will set the proper setting to cfg variable

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char *)cfg["ssid"], (const char *)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : ");
    Serial.println(WiFi.localIP());
    // USER CODE EXAMPLE : meta data to local variable
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? atoi((const char *)meta["pubInterval"]) : 0;
    lastPublishMillis = -pubInterval;

    set_iot_server();
    client.setCallback(message);
    iot_connect();
}

void loop()
{
    if (!client.connected())
    {
        iot_connect();
    }

    client.loop();

    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval))
    {
        publishData();
        lastPublishMillis = millis();
    }
}