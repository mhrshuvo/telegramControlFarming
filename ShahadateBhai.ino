
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

DHT dht(2, DHT11);
const int AirValue = 790;   //you need to replace this value with Value_1
const int WaterValue = 390;  //you need to replace this value with Value_2
const int soilMost = A0;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;
const int pumpO = 5;
int pumpOstatus = 0;
const int pumpT = 4;
int pumpTstatus = 0;
#define WIFI_SSID "thesis"
#define WIFI_PASSWORD "password"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "***********************************************"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done



void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/temperature") {
      String readings = getTemperature();
      bot.sendMessage(chat_id, readings, "");
    }
    if (text == "/humidity") {
      String readings = getHumidity();
      bot.sendMessage(chat_id, readings, "");
    }
    if (text == "/soilMost") {
      soilMoistureValue = analogRead(soilMost);  //put Sensor insert into soil
      Serial.println(soilMoistureValue);
      soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
      if (soilmoisturepercent > 100)
      {
        Serial.println("100 %");


        bot.sendMessage(chat_id, "Soil Mosture: 100 %  \n", "");

      }
      else if (soilmoisturepercent < 0)
      {
        Serial.println("0 % ");


        bot.sendMessage(chat_id, "Soil Mosture: 0 %  \n", "");
      }

    }
    if (text == "/P1Status") {
      if (pumpOstatus)
      {
        bot.sendMessage(chat_id, "PUMP One is ON", "");
      }
      else
      {
        bot.sendMessage(chat_id, "PUMP One is OFF", "");
      }
    }
      if (text == "/P2Status") {
      if (pumpTstatus)
      {
        bot.sendMessage(chat_id, "PUMP Two is ON", "");
      }
      else
      {
        bot.sendMessage(chat_id, "PUMP Two is OFF", "");
      }
    }
    if (text == "/P1ON") {
      pumpOstatus = 1;
      digitalWrite(pumpO, LOW);
      bot.sendMessage(chat_id, "PUMP One turned  ON", "");
    }
    if (text == "/P2ON") {
      pumpTstatus = 1;
      digitalWrite(pumpT, LOW);
      bot.sendMessage(chat_id, "PUMP Two turned  ON", "");
    }
     if (text == "/P1OFF") {
      pumpOstatus = 1;
      digitalWrite(pumpO, HIGH);
      bot.sendMessage(chat_id, "PUMP One turned  OFF", "");
    }
    if (text == "/P2OFF") {
      pumpTstatus = 1;
      digitalWrite(pumpT,HIGH);
      bot.sendMessage(chat_id, "PUMP Two turned  OFF", "");
    }

    if (text == "/start")
    {
      String welcome = "Welcome to my thesis project, " + from_name + ".\n";
      welcome += "This is automatic irrigation system.\n\n";
      welcome += "/temperature : know about the temperature \n";
      welcome += "/humidity : know about the humidity\n";
      welcome += "/soilMost : know about the soil mosture\n";
      welcome += "/P1Status : know about the Pump one status\n";
      welcome += "/P2Status : know about the Pump two status\n";
      welcome += "/P1ON : Turn on pump one\n";
      welcome += "/P2ON : Turn on pump Two\n";
      welcome += "/P1OFF : Turn off pump one\n"; 
      welcome += "/P2OFF : Turn off pump one\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(pumpO, OUTPUT);
  pinMode(pumpT, OUTPUT);
  digitalWrite(pumpO, HIGH);
  digitalWrite(pumpT, HIGH);
  //  pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
  //  delay(10);
  //  digitalWrite(ledPin, HIGH); // initialize pin as off (active LOW)

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

String getTemperature() {
  float temperature;
  temperature = dht.readTemperature();
  String message = "Temperature: " + String(temperature) + " ºC \n";
  return message;
}

String getHumidity() {
  float humidity;
  humidity = dht.readHumidity();
  String message = "Humidity: " + String (humidity) + " % \n";
  return message;
}
