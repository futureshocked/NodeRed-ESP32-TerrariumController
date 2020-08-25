/*  Node-RED and ESP32 Make a Terrarium Controller
 * 
 * This code is part of a course from Tech Explorations.
 * For information about this course, please see
 * 
 * https://techexplorations.com/so/nodered/
 * 
 * For information on hardware components and the wiring needed to 
 * run this sketch, please see the relevant lecture in the course.
 * 
 *  
 *  Created by Peter Dalmaris
 * 
 */

#include "EspMQTTClient.h"
#include "esp32_secrets.h"

EspMQTTClient client(
  SECRET_SSID,
  SECRET_PASS,
  BROKER_IP,         // MQTT Broker server ip
  BROKER_USERNAME,   // Can be omitted if not needed
  BROKER_PASSWORD,   // Can be omitted if not needed
  CLIENT_NAME,       // Client name that uniquely identify your device
  BROKER_PORT        // The MQTT port, default to 1883. this line can be omitted
);

const byte soil_humidity_pin  = 35;
const byte pump_pin           = 32;

String soil_humidity_topic  = "techexplorations/terrarium/soil-humidity";
String pump_control_topic   = "techexplorations/terrarium/pump-control";

int soil_humidity_error_range = 10; // Updates to the broker will be sent only 
                                    // if new soil humidity differs from last
                                    // soil humidity by soil_humidity_error_range.
int current_soil_humidity;
int last_soil_humidity = -1;        // The initial value.      

void setup()
{
  //adcAttachPin(soil_humidity_pin);
  analogReadResolution(10);
  pinMode(soil_humidity_pin, INPUT);
  pinMode(pump_pin, OUTPUT);
  Serial.begin(115200);

  // Optionnal functionnalities of EspMQTTClient :
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater();    // Enable the web updater. 
                                    // User and password default to values of MQTTUsername 
                                    // and MQTTPassword. These can be overrited with 
                                    // enableHTTPWebUpdater("user", "password").

}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  client.subscribe(pump_control_topic, [](const String & pump_state) {
      pump_control(pump_state);
  });
}

void loop()
{
  client.loop(); // must be called once per loop.

  current_soil_humidity = analogRead(soil_humidity_pin);
  Serial.print("Current: ");
  Serial.print(current_soil_humidity);
  Serial.print(", last: ");
  Serial.print(last_soil_humidity);
  Serial.print(", diff: ");
  Serial.print(abs(current_soil_humidity - last_soil_humidity));
   
  // Is the current soil humidity different enough from the last one to do an update?
  if ( abs(current_soil_humidity - last_soil_humidity) > soil_humidity_error_range )
  {
      Serial.print(", Sending... ");
      Serial.print(analogRead(soil_humidity_pin));
    
      client.publish(soil_humidity_topic, String(current_soil_humidity, DEC));
      last_soil_humidity = current_soil_humidity;
      Serial.println();
  }
  Serial.println();
  delay(1000);
}

void pump_control(String pump_state)
{
  Serial.println(pump_state);
    if (pump_state[0] == '0')
    {
      Serial.println("Stop pump");
      digitalWrite(pump_pin, LOW);
    } else if (pump_state[0] == '1')
    {
      Serial.println("Start pump");
      digitalWrite(pump_pin, HIGH);
    }
}
