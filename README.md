# Node-RED and ESP32: make a terrarium controller

Flows, ESP32 sketches and the KiCad project for the Tech Explorations course [Node-RED and ESP32: make a terrarium controller](https://app.techexplorations.com/courses/node-red-esp32-make-a-terrarium-controller/).

## Project status (October 2026)

The course was recorded in 2020 with Node-RED 1.1 and Node.js 12. The project still works on current software: flow v6 was tested unchanged on Node-RED 5.0.8 with Node.js 22 and 26. The Node-RED editor, the install process and some third-party services have changed since recording.

- **Following the course now?** Read [UPDATE-2026.md](UPDATE-2026.md) first. It covers installing Node-RED 5, the new editor, the deprecated dashboard, Mosquitto 2, and services that have changed (IFTTT webhooks now need a paid plan).
- **What comes next:** [COURSE-UPDATE-PLAN.md](COURSE-UPDATE-PLAN.md) describes the planned revision, including replacing the DHT22, Google Sheets and IFTTT with open-source alternatives.

## What the project does

An ESP32 in the terrarium reads soil moisture, air temperature and humidity, and the voltages of the pump motor supply and its own battery. It publishes these readings over MQTT to a Mosquitto broker on a Raspberry Pi. Node-RED on the Pi subscribes to the readings, shows them on a web dashboard, decides when the soil is dry enough to run the water pump, and sends the pump command back to the ESP32 over MQTT. Later versions add a dashboard control for the moisture threshold, data logging and a low-battery alert.

## Repository contents

### Terrarium controller flows

Each version builds on the previous one, following the course.

| File | Adds |
|---|---|
| `TerrariumController-Flow-v1.json` | Soil moisture gauge and pump control over MQTT |
| `TerrariumController-Flow-v2.json` | Motor and MCU battery voltage gauges |
| `TerrariumController-Flow-v3.json` | Logging to Google Sheets |
| `TerrariumController-Flow-v4.json` | Dashboard control for the soil moisture threshold |
| `TerrariumController-Flow-v5.json` | Air temperature and humidity gauges |
| `TerrariumController-Flow-v6.json` | Low MCU battery alert through IFTTT |

### ESP32 sketches

`ESP32-TerariumControl-v1` to `v6` are the matching sketches. Each folder has an `esp32_secrets.h` file; fill in your Wi-Fi and MQTT broker details before uploading. The sketches need these libraries: EspMQTTClient, TaskScheduler (from v4) and DHTesp (v6).

### Example flows

Smaller flows used in the early lectures to introduce Node-RED concepts:

- Core nodes: `Inject-Function-flow.json`, `Switch-flow.json`, `Global-Variables-context-examples-flows.json`
- Dashboard widgets: `Basic-dashboard-example-flow.json`, `Dashboard-example-flow.json`, `Button-flow.json`, `Dropdown-template-flow.json`, `Form-flow.json`, `gauge-flow.json`
- Messaging and web services: `mqtt-example-flows.json`, `websockets-flow.json`, `dweetio-flow.json`, `ifttt-flow.json`
- DHT22 sensor on the Raspberry Pi: `DHT22-ex1-flow.json`, `DHT22-ex2-flow.json`, `DHT22-complete-example-flow.json`

### Hardware

`KiCad Terrarium Controller project/` contains the schematic and PCB for the controller board (KiCad 9 format), a PDF of the original schematic, and Gerber files. The LED orientation in the schematic was corrected in November 2025.

## Requirements

- Raspberry Pi 3, 4 or 5 with a 64-bit OS (or any computer that runs Node-RED)
- Node-RED 5 on Node.js 24 LTS (Node.js 22.9 or later is required; 26 also works)
- Mosquitto MQTT broker
- `node-red-dashboard` installed from the Node-RED Palette Manager
- ESP32 development board and the components listed in the course

## Importing a flow

In the Node-RED editor, open the main menu, choose Import, and paste the contents of a flow file or select the file. Install `node-red-dashboard` first, otherwise the dashboard nodes appear as "unknown". After importing, open the MQTT broker node and set your broker address, username and password.

## Help

Questions about the course go in the course Q&A. When reporting a problem, include the lecture number, your Node-RED and Node.js versions, and the exact error message.

## Licence

MIT, see [LICENSE](LICENSE).
