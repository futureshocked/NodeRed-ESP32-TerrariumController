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

// This version makes it possible to publish voltage levels. 

#include "EspMQTTClient.h"
#include "esp32_secrets.h"

const float   input_voltage_max               = 9.0; // In Volts, max voltage 
const int     R1_motor                        = 303; // Ohms, in kOhm
const int     R2_motor                        = 220; // Ohms, in kOhm
const float   correction_factor               = 0.95; //1.62; // Helps improve the accuracy of the voltage calculation
const float   max_voltage_on_analog_input     = 3.6; //2.49;
const byte    motor_voltage_sensor_pin        = 34;

const float   mcu_battery_max                 = 4.7;
const int     R1_mcu                          = 220; // Ohms, in kOhm
const int     R2_mcu                          = 420; // Ohms, in kOhm
const float   correction_factor_mcu           = 1.0; //1.62; // Helps improve the accuracy of the voltage calculation
const float   max_voltage_on_analog_input_mcu = 3.1; // calculated using the voltage divider calculator
const byte    mcu_voltage_sensor_pin          = 33;


EspMQTTClient client(
  SECRET_SSID,
  SECRET_PASS,
  BROKER_IP,         // MQTT Broker server ip
  BROKER_USERNAME,   // Can be omitted if not needed
  BROKER_PASSWORD,   // Can be omitted if not needed
  CLIENT_NAME,       // Client name that uniquely identify your device
  BROKER_PORT        // The MQTT port, default to 1883. this line can be omitted
);

const byte  soil_humidity_pin           = 35;
const byte  pump_pin                    = 32;

String      soil_humidity_topic         = "techexplorations/terrarium/soil-humidity";
String      pump_control_topic          = "techexplorations/terrarium/pump-control";
String      motor_voltage_topic         = "techexplorations/terrarium/motor_voltage";
String      mcu_voltage_topic           = "techexplorations/terrarium/mcu_voltage";

int         soil_humidity_error_range   = 10; // Updates to the broker will be sent only 
                                              // if new soil humidity differs from last
                                              // soil humidity by soil_humidity_error_range.
int         current_soil_humidity;
int         last_soil_humidity          = -1;        // The initial value.      

void setup()
{
  analogReadResolution(10);
  pinMode(soil_humidity_pin, INPUT);
  pinMode(pump_pin, OUTPUT);
  Serial.begin(115200);

  // Optionnal functionnalities of EspMQTTClient :
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").

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
      client.publish(soil_humidity_topic, String(analogRead(soil_humidity_pin)) + "," + CLIENT_NAME);
      last_soil_humidity = current_soil_humidity;
      Serial.println();
  }
  Serial.println();
  report_voltages();
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

void report_voltages()
{
  float motor_voltage = motor_battery_voltage();
  float mcu_voltage   = mcu_battery_voltage();

  Serial.print("Motor voltage: ");
  Serial.print(motor_voltage);
  Serial.print(", MCU voltage: ");
  Serial.println(mcu_voltage);

  client.publish(motor_voltage_topic, String(motor_voltage, 1));
  client.publish(mcu_voltage_topic,   String(mcu_voltage, 1));
}

float motor_battery_voltage()
{
  // read the input on analog pin 0:
  int sensorValue = analogRead(motor_voltage_sensor_pin);
  // print out the value you read:
  Serial.print("Motor Voltage sensor raw: ");
  Serial.print(sensorValue);
  // This formula will convert the raw analog input reading to an approximate voltage value.
  float battery_voltage = (sensorValue * max_voltage_on_analog_input * correction_factor / 1023) * (R1_motor + R2_motor) / R2_motor;
  Serial.print(", voltage = ");
  Serial.println(battery_voltage);
  return battery_voltage;
}

float mcu_battery_voltage()
{
  // read the input on analog pin 0:
  int sensorValue_mcu = analogRead(mcu_voltage_sensor_pin);
  // print out the value you read:
  Serial.print("MCU Voltage sensor raw: ");
  Serial.print(sensorValue_mcu);
  // This formula will convert the raw analog input reading to an approximate voltage value.
  float mcu_battery_voltage = (sensorValue_mcu * max_voltage_on_analog_input_mcu * correction_factor_mcu / 1023) * (R1_mcu + R2_mcu) / R2_mcu;
  Serial.print(", voltage = ");
  Serial.println(mcu_battery_voltage);
  return mcu_battery_voltage;
}
