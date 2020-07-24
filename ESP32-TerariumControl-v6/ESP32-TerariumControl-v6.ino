// This version introduces the DHT22 sensor

#include "EspMQTTClient.h"
#include "esp32_secrets.h"
#include <TaskScheduler.h>  // See https://github.com/arkhipenko/TaskScheduler
#include "DHTesp.h"         // See https://github.com/beegee-tokyo/DHTesp

// The sensor is affected by interference from the motor when it is working.
// Add a ceramic capacitor ~100nF across the motor terminals to suppress the noise.
// See here: https://www.electroschematics.com/wp-content/uploads/2015/02/arduino-dht22-circuit.png
DHTesp dht;
int dhtPin = 25;

float air_sensor[2]; // This array contains the air temperature and humidity values.

void report_soil_humidity_CB();
Task report_soil_humidity(1000, TASK_FOREVER, &report_soil_humidity_CB);

void pump_off_CB();
Task turn_pump_off(5000, TASK_FOREVER, &pump_off_CB);

void report_voltages_CB();
Task report_voltages(2500, TASK_FOREVER, &report_voltages_CB);

void report_air_sensor_values_CB();
Task report_air_sensor_values(2500, TASK_FOREVER, &report_air_sensor_values_CB);

Scheduler runner;

const float   input_voltage_max               = 6.0; // In Volts, max voltage
const int     R1                              = 303; // Ohms, in kOhm
const int     R2                              = 220; // Ohms, in kOhm
const float   correction_factor               = 1.45; //1.62; // Helps improve the accuracy of the voltage calculation
const float   max_voltage_on_analog_input     = 2.49;
const byte    motor_voltage_sensor_pin        = 34;  // GPIO

const float   mcu_battery_max                 = 4.2;
const int     R1_mcu                          = 220; // Ohms, in kOhm
const int     R2_mcu                          = 420; // Ohms, in kOhm
const float   correction_factor_mcu           = 1.19; //1.62; // Helps improve the accuracy of the voltage calculation
const float   max_voltage_on_analog_input_mcu = 2.81; // calculated using the voltage divider calculator
const byte    mcu_voltage_sensor_pin          = 33;  // GPIO

int           soil_humidity_threshold         = 400; // This is the initial value for the threshold. Actual value set via MQTT.

EspMQTTClient client(
  SECRET_SSID,
  SECRET_PASS,
  BROKER_IP,        // MQTT Broker server ip
  BROKER_USERNAME,  // Can be omitted if not needed
  BROKER_PASSWORD,  // Can be omitted if not needed
  CLIENT_NAME,      // Client name that uniquely identify your device
  BROKER_PORT       // The MQTT port, default to 1883. this line can be omitted
);

const byte  soil_humidity_pin           = 35; // GPIO
const byte  pump_pin                    = 32; // GPIO

String      soil_humidity_topic         = "techexplorations/terrarium/soil-humidity";
String      pump_control_topic          = "techexplorations/terrarium/pump-control";
String      motor_voltage_topic         = "techexplorations/terrarium/motor_voltage";
String      mcu_voltage_topic           = "techexplorations/terrarium/mcu_voltage";
String      get_soil_humidity_threshold = "techexplorations/terrarium/soil_humidity_threshold";
String      air_humidity_topic          = "techexplorations/terrarium/air-humidity";
String      air_temperature_topic       = "techexplorations/terrarium/air-temperature";

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

  dht.setup(dhtPin, DHTesp::DHT22);

  Serial.begin(115200);

  // Optionnal functionnalities of EspMQTTClient :
  //client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").

  runner.init();
  runner.addTask(report_soil_humidity);
  runner.addTask(report_voltages);
  runner.addTask(turn_pump_off);
  runner.addTask(report_air_sensor_values);

  report_soil_humidity.enable();
  report_voltages.enable();
  report_air_sensor_values.enable();
  //turn_pump_off.enable();  // Turn on the pump task only after the pump is turned on
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  client.subscribe(pump_control_topic, [](const String & pump_state) {
    pump_control(pump_state);
  });

  client.subscribe(get_soil_humidity_threshold, [](const String & threshold) {
    set_threshold(threshold);
  });
}

void loop()
{
  client.loop(); // must be called once per loop.
  runner.execute();
  //  report_soil_humidity();
  //  delay(1000);    // No longer needed as we are using the scheduler.
}

void report_air_sensor_values_CB()
{
  //float air_sensor[2];
  get_air_sensor_values(&air_sensor[0]);

  Serial.print("Air humidity: ");
  Serial.print(String(air_sensor[1], 2));
  Serial.print(", air temperature: ");
  Serial.println(String(air_sensor[0], 2));

  client.publish(air_humidity_topic, String(air_sensor[1], 2));
  client.publish(air_temperature_topic, String(air_sensor[0], 2));
}

void report_soil_humidity_CB()  // This function formerly named 'report_soil_humidity'. 'CB' stands for CallBack
{
  current_soil_humidity = analogRead(soil_humidity_pin);
  Serial.print("Current: ");
  Serial.print(current_soil_humidity);
  Serial.print(", last: ");
  Serial.print(last_soil_humidity);
  Serial.print(", diff: ");
  Serial.print(abs(current_soil_humidity - last_soil_humidity));

  // Is the current soil humidity different enough from the last one to do an update?
  if ( abs(current_soil_humidity - last_soil_humidity) > soil_humidity_error_range  || current_soil_humidity > soil_humidity_threshold )
  {
    Serial.print(", Sending... ");
    Serial.println(analogRead(soil_humidity_pin));

    client.publish(soil_humidity_topic, String(analogRead(soil_humidity_pin)) + "," + CLIENT_NAME);
    last_soil_humidity = current_soil_humidity;
    Serial.println();
  }
  Serial.println();
  //  report_voltages();  // Not needed as now using the Scheduler to call this function.
}

void set_threshold(String threshold)
{
  soil_humidity_threshold = threshold.toInt();
  Serial.print("Threshold: ");
  Serial.println(soil_humidity_threshold);
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
    turn_pump_off.enableDelayed(5000);  // Automatically turn pump off after 5 seconds
  }
}

void report_voltages_CB()   // Formerly titled 'report_voltages'
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
  float battery_voltage = (sensorValue * max_voltage_on_analog_input * correction_factor / 1023) * (R1 + R2) / R2;
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
  float mcu_battery_voltage = (sensorValue_mcu * max_voltage_on_analog_input_mcu * correction_factor_mcu / 1023) * (R1_mcu + R2_mcu) / R2_mcu;
  Serial.print(", voltage = ");
  Serial.println(mcu_battery_voltage);
  return mcu_battery_voltage;
}

void pump_off_CB()
{
  Serial.println("Stop pump CB");
  digitalWrite(pump_pin, LOW);
  turn_pump_off.disable();
}

void get_air_sensor_values(float *air_sensor)
{
  TempAndHumidity newValues = dht.getTempAndHumidity();
  air_sensor[0] = newValues.temperature;
  air_sensor[1] = newValues.humidity;
}
