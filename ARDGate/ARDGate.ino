#include <SPI.h>           // Ethernet shield
#include <Ethernet.h>      // Ethernet shield
#include <PubSubClient.h>  // MQTT 
#include <dht11.h>

#define DHT11_PIN 8
#define LED_State 13
#define ch1_pin 3
#define ch2_pin 4
#define ch3_pin 5
#define ch4_pin 6


//#define DEBUG

dht11 DHT;

int h_value = 0;         // показания датчика влажности
int t_value = 0;         // показания датчика температуры

byte mac[]    = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x02 };
byte server[] = { 192, 168, 1, 241 };
byte ip[]     = { 192, 168, 1, 230 };

char buff_msg[32];            // mqtt message

void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(ethClient);
unsigned long lastMqtt = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strTopic);

  if (strTopic == "/gate1/rl/ch1") {
    if (strPayload == "OFF") digitalWrite(ch1_pin, HIGH);
    else if (strPayload == "ON") digitalWrite(ch1_pin, LOW);
  } 
  if (strTopic == "/gate1/rl/ch2") {
    if (strPayload == "OFF") digitalWrite(ch2_pin, HIGH);
    else if (strPayload == "ON") digitalWrite(ch2_pin, LOW);
  }
  if (strTopic == "/gate1/rl/ch3") {
    if (strPayload == "OFF") digitalWrite(ch3_pin, HIGH);
    else if (strPayload == "ON") digitalWrite(ch3_pin, LOW);
  }
  if (strTopic == "/gate1/rl/ch4") {
    if (strPayload == "OFF") digitalWrite(ch4_pin, HIGH);
    else if (strPayload == "ON") digitalWrite(ch3_pin, LOW);
  }
}

void setup() {
#ifdef DEBUG
  Serial.begin(57600);
  delay(500);
  Serial.println("Start gateway");
#endif

  pinMode(LED_State, OUTPUT);
  digitalWrite(LED_State, HIGH);
  delay(1000);
  digitalWrite(LED_State, LOW);
  delay(1000);
  digitalWrite(LED_State, HIGH);

  client.setServer(server, 1883);
  client.setCallback(callback);

  pinMode(ch1_pin, OUTPUT); 
  pinMode(ch2_pin, OUTPUT); 
  pinMode(ch3_pin, OUTPUT); 
  pinMode(ch4_pin, OUTPUT); 
  
  digitalWrite(ch1_pin, HIGH);
  digitalWrite(ch2_pin, HIGH);
  digitalWrite(ch3_pin, HIGH);
  digitalWrite(ch4_pin, HIGH);
  

  Ethernet.begin(mac, ip);
  if (client.connect("gw1Client")) {
    client.subscribe("/gate1/rl/#");
#ifdef DEBUG
    Serial.println("Setup client conected...");
#endif
  }
  else
  {
#ifdef DEBUG
    Serial.println("OffLine");
#endif
  }
  delay(1500);
}

void loop() {
  
  int chk;
  chk = DHT.read(DHT11_PIN);    // READ DATA
#ifdef DEBUG  
  switch (chk){
      case DHTLIB_OK:  
                //Serial.println("Ok");                
                break;
      case DHTLIB_ERROR_CHECKSUM: 
                Serial.println("Checksum error"); 
                break;
      case DHTLIB_ERROR_TIMEOUT: 
                Serial.println("Time out error"); 
                break;
      default: 
                Serial.println("Unknown error"); 
                break;
  }
#endif    
  
  h_value = DHT.humidity;
  t_value = DHT.temperature;

#ifdef DEBUG
  Serial.print("Hum: ");
  Serial.print(String(h_value));
  Serial.print("  temp: ");
  Serial.println(t_value);
#endif 
    
  if (lastMqtt > millis()) lastMqtt = 0;
  client.loop();

  // здесь какой-то другой код по уравлению светом, например, с кнопок или ещё как 
  
  if (millis() > (lastMqtt + 10000)) {
    if (!client.connected()) {
      if (client.connect("gw1Client")) {
        client.subscribe("/gate1/rl/#");
       }
    } 
    if (client.connected()) 
    { 
      sprintf(buff_msg, "%d", h_value);
      client.publish("/gate1/humidity1", buff_msg);
      sprintf(buff_msg, "%d", t_value);
      client.publish("/gate1/temp1", buff_msg);
#ifdef DEBUG
      Serial.println("send data to mqtt broker");
#endif      
    }    
    lastMqtt = millis();
  }
  delay(1000);
}
