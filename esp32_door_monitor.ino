#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; // India gmt offset = +5:30
const int   daylightOffset_sec = 0;
struct tm openTimeInfo;
String angry_photo_url = "https://media0.giphy.com/media/11tTNkNy1SdXGg/200.gif";

String getOpenDateTime()
{
  if (!getLocalTime(&openTimeInfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }

  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &openTimeInfo);
  //print like "const char*"
  Serial.println(timeStringBuff);

  return timeStringBuff;
}


#define DOOR_SENSOR_PIN  4  // ESP32 pin G4 connected to door sensor pin

int doorState = 0;
String state = "closed";

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Initialize Telegram BOT
#define BOTtoken "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" // bot token
#define CHAT_ID "XXXXXXXXX" //chat id

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void setup() {
  Serial.begin(115200);                     // initialize serial
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);   // set ESP32 pin to input pull-up mode

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  bot.sendMessage(CHAT_ID, "Bot started up", "");
}

void loop() {
  doorState = digitalRead(DOOR_SENSOR_PIN); // read state

  if (doorState == LOW && state == "closed") {
    state = "open";
    //Send notification
    bot.sendMessage(CHAT_ID, "Door " + state + " at " + getOpenDateTime());

    Serial.println(state);
    Serial.println(doorState);
  }

    time_t now;
    time(&now);  /* get current time; same as: now = time(NULL)  */
    int seconds = difftime(now, mktime(&openTimeInfo));
    if ((seconds%60) == 0 && state == "open"){
      bot.sendPhoto(CHAT_ID, angry_photo_url, "You didn't close the door on time!");
    }
    

  if (doorState == HIGH && state == "open") {
    state = "closed";

    time_t now;
    time(&now);  /* get current time; same as: now = time(NULL)  */
    double seconds = difftime(now, mktime(&openTimeInfo));

    //Send notification
    bot.sendMessage(CHAT_ID, "Door " + state + " after " + seconds + "sec");
    
    }

    Serial.println(state);
    Serial.println(doorState);
  }
