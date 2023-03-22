//calling librairies
#include <ESP8266WiFi.h> //this librairy handle connexion with local network
#include <PubSubClient.h> //this librairy handle MQTT protocol between the nodeMCU(client) and raspbery(server) 
#include "DHT.h"  //this librairy responsible to menage the DHT sensor

#define MSG_BUFFER_SIZE  (50)

#define BUILTIN_LED //D1 specify the pin number that is binded to lamp
 
DHT dht(D3, DHT11); //D3 : this is the pin number on the nodeMCU board to which the DHT sensor data is connected. inthis case, pin D3
                    //DHT11 : this the type of DHT sensor being used.
float temp; //holds the value catched of temperature sensor.
float humi; //holds the value catched of humidity

const char* ssid = "WIFI_ssid";  //specify the name of the network to connect with/
const char* password = "WIFI_pass";
const char* mqtt_server = "MQTT_SERVER"; 


//global variables used in our program
/*
creates a wifiClient object naed, "espclient", which will be used to connect to a wifi acsses point, then it creates a PubSubClient object named 
"client" which takes the wifiClient object "espclient" as a parameter. this PubSubClient object will be used to connect an MQTT
broker and publish or subscribe to message
*/
WiFiClient espClient;
PubSubClient client(espClient);

char msg[MSG_BUFFER_SIZE];  //takes the topic from MQTT broker

//establish the connection with the network 
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //loop is turning while there is no connectionestablished
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros()); /* enabling the random number generator, the randomSeed fi=unction is called to initialise the random number generator then,
  the random() function is used to generate a random number between 0 and returnedvalue of micro() function which returns the number of milliseconds 
  since th board began running. we will need it in th line 98 
  */
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
/*
when an event or message triggers a callback function, it is executed by the MQTT client library. this allow the user to perform custom actions
in response to MQTT events and messages.  
*/
void callback(char* topic, byte* payload, unsigned int length) {
  //topic : the field that specifies which component will be triggered
  //message : specifies the action performed by this component
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]); // retrieves the message recieved and casting it from byte type to string
  }
  Serial.println();
  
  //if temperature is required 
  if ((char)payload[0] == 'T') {
    char temperature[6];
    temp = dht.readTemperature();   // retrieve the temperature from the sensor
    sprintf(temperature, "%.f", temp);
    client.publish("homeex/temperature",temperature);  
    /*
      the value is read and sent to the MQTT broker with the publish function under the topic homeex/temperature and the message carries the
    the value read by the sensor
    */
  
  //if humidity is required 
  }else if((char)payload[0] == 'H'){ 
    char humidity[6];
    humi = dht.readHumidity();  // retrieve the humidity from the sensor
    sprintf(humidity, "%.f", humi);
    client.publish("homeex/humidity",humidity);
        /*
      the value is read and sent to the MQTT broker with the publish function under the topic homeex/humidity and the message carries the
    the value read by the sensor
    */
  //the message 1 for turning on the light
  //the message 0 for turning off
  }else  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }else if ((char)payload[0] == '0'){
    digitalWrite(BUILTIN_LED, LOW);  // Turn the LED off by making the voltage HIGH
  }

}

/*the reconnect function is usually implimented by the MQTT client librairy and can be configured to automatically
attempt to reconnect to the broker with a specified intervall between each attempt.
*/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("homeex/nodemcu");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  dht.begin(); //enable the DHT sensor
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  /*
  sets the broker that the client will connect to it takes two arguments  
  mqtt_server : this is a character array or string that specifies the IP adress or domain name of the MQTT broker.
  1883 : is the port number on which the MQTT broker is listining to for connection.
  */
  client.setCallback(callback); //sets the callback function that will be called when a lessage is recieved from MQTT broker.
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();  /* the loop function is used to maintain the MQTT client's connection with the broker and to process incoming
  and outgoing messages.
  */

}