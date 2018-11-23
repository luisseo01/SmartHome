/*
 * SmartHome 프로젝트
 * 2구 스위치 버전
 * 거실방
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT11.h>
//-----Publish 토픽1
#define __Pub_TOPIC1__ "/Room/Main/RE1"
//-----Publish 토픽2
#define __Pub_TOPIC2__ "/Room/Main/RE2"

//-----Publish 토픽 DHT11 온도
#define __Pub_DHT11_TM_TOPIC2__ "/Room/Main/TM"
//-----Publish 토픽 DHT11 습도
#define __Pub_DHT11_HU_TOPIC2__ "/Room/Main/HU"

//-----Subscribe 토픽
#define __Sub_TOPIC__ "/Room/Main"

//-----모듈 넘버
#define __Client_Name__ "ESP_Room_Main"

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";



//-----핀아웃 설정
int SW1 = 12;
int SW2 = 13;
int LL1 = 5;
int LL2 = 4;
int DHT = 2;
int GAS = 14;
////////////////
//-----릴레이 설정
int Togle1 = 0;
int Togle2 = 0;
////////////////
//-----DHT11 설정
DHT11 dht11(DHT);
long long Timer_DHT11 = 0;
////////////////
//-----WIFI, MQTT 설정
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
////////////////

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) { //정보 도착하면 실행
  Serial.print("Topic:\"");
  Serial.print(topic);
  Serial.print("\" -->");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /* if문 시작 */
  if ((char)payload[0] == '1') {
    digitalWrite(LL1, LOW);
    Togle1 = 0;
  }
  if ((char)payload[0] == '0') {
    digitalWrite(LL1, HIGH);
    Togle1 = 1;
  }
  if ((char)payload[0] == '4') {
    digitalWrite(LL2, LOW);
    Togle2 = 0;
  }
  if ((char)payload[0] == '3') {
    digitalWrite(LL2, HIGH);
    Togle2 = 1;
  }
}

void reconnect() { //연결 끊기면 다시 연결
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection..."); //연결중
    // Attempt to connect
    if (client.connect(__Client_Name__)) {
      Serial.println("연결됨"); //

      //client.publish("test", "hello world");

      /* Subscribe할 토픽이름 */
      client.subscribe(__Sub_TOPIC__);

    } else { //연결 실패시
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/*
 * 16 1번 스위치
 * 05 2번 스위치
 * 04 1번 릴레이
 * 00 2번 릴레이
 * A0 DHT11
 * 14 가스감지
 */
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(DHT, INPUT);
  pinMode(GAS, INPUT);
  pinMode(LL1, OUTPUT);
  pinMode(LL2, OUTPUT);
  digitalWrite(LL1, HIGH);
  digitalWrite(LL2, HIGH);
}

int togle_SW1 = 0;
int togle_SW2 = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  /*if (!digitalRead(SW1)) {
    Togle1 = !Togle1;
    digitalWrite(LL1, Togle1);
    snprintf (msg, 75, "%d", !Togle1);
    client.publish(__Pub_TOPIC1__, msg);
    delay(200);
  }*/
  if (!digitalRead(SW1)) togle_SW1 = 1;
  if (digitalRead(SW1) && togle_SW1) {
    togle_SW1=0;
    Togle1 = !Togle1;
    digitalWrite(LL1, Togle1);
    snprintf (msg, 75, "%d", !Togle1);
    client.publish(__Pub_TOPIC1__, msg);
  }

  /*if (!digitalRead(SW2)) {
    Togle2 = !Togle2;
    digitalWrite(LL2, Togle2);
    snprintf (msg, 75, "%d", !Togle2 + 3);
    client.publish(__Pub_TOPIC2__, msg);
    delay(200);
  }*/
  if (!digitalRead(SW2)) togle_SW2 = 1;
  if (digitalRead(SW2) && togle_SW2) {
    togle_SW2 = 0;
    Togle2 = !Togle2;
    digitalWrite(LL2, Togle2);
    snprintf (msg, 75, "%d", !Togle2 + 3);
    client.publish(__Pub_TOPIC2__, msg);
  }

  if (Timer_DHT11 < millis()) {
    int err;
    float temp, humi;
    if ((err = dht11.read(humi, temp)) == 0) {
      //Serial.println("asdf");//asdfasdf
      snprintf (msg, 75, "%d", (int)temp);
      client.publish(__Pub_DHT11_TM_TOPIC2__, msg);
      snprintf (msg, 75, "%d", (int)humi);
      client.publish(__Pub_DHT11_HU_TOPIC2__, msg);
    }
    Timer_DHT11 = millis() + 10000; //10초에 한번씩
  }
  client.loop();
}
