# Course update plan

Working plan for the next revision of *Node-RED and ESP32: make a terrarium controller*. This is a planning document, not student instructions; for what works today, see [UPDATE-2026.md](UPDATE-2026.md).

Status: draft, October 2026. Nothing below has been built or recorded yet.

## Why update

The project still runs on Node-RED 5 and Node.js 26 (see UPDATE-2026.md), so the update is not a rescue. It addresses three things:

1. The videos no longer match what students see when they install Node-RED.
2. Three parts of the project depend on things that have aged badly: the DHT22 sensor, Google Sheets for logging, and IFTTT for alerts.
3. The dashboard uses `node-red-dashboard`, which was deprecated in 2024.

The aim is a project that a student can build end to end with free, open-source tools running on their own Raspberry Pi, with no third-party accounts required.

## 1. Air sensor: replace the DHT22

### Problem

The DHT22 uses a timing-sensitive single-wire protocol. The v6 sketch already carries a comment that the pump motor corrupts its readings unless a capacitor is added across the motor. Its humidity accuracy (about ±2 to 5 %RH) is modest, and terrarium air sits near saturation, where cheap humidity sensors drift or saturate. The DHTesp library used in the sketch is marked by its author as no longer maintained. On the Raspberry Pi side, the DHT22 node needs native code that does not build on a Pi 5 without the libgpiod workaround.

### Options

| Sensor | Interface | Notes |
|---|---|---|
| Sensirion SHT40 / SHT41 / SHT45 | I2C | Better accuracy than the DHT22; has a built-in heater that can drive off condensation, which suits a terrarium. Maintained libraries (for example Adafruit SHT4x, Sensirion's own). Widely sold on breakout boards. |
| Bosch BME280 | I2C or SPI | Adds pressure. Humidity accuracy similar to SHT4x. Many cheap boards are actually the BMP280 (no humidity), so sourcing needs care. |
| AHT20 | I2C | Cheapest I2C option; acceptable accuracy; less established in high humidity. |

### Recommendation

Move to an SHT4x breakout over I2C. I2C removes the timing problem, the heater is a real teaching point for a high-humidity enclosure, and it drops the unmaintained DHTesp library. The MQTT topics (`air-temperature`, `air-humidity`) stay the same, so the Node-RED side does not change.

Drop the Raspberry Pi DHT22 examples (`DHT22-ex1/ex2/complete`) or replace them with an I2C sensor read from the Pi, which works on every Pi model without native GPIO libraries.

### To check before recording

- Confirm the chosen breakout is easy to buy in AU, US, EU and from AliExpress.
- Confirm the current soil moisture probe type. If it is a resistive probe, consider a capacitive one at the same time; resistive probes corrode in permanently wet soil.

## 2. Data logging: replace Google Sheets

### Problem

The Google Sheets node (`node-red-contrib-google-sheets`) has not been updated since 2022. Setting it up requires a Google Cloud project, an enabled API, a service account and a JSON key, which is a long detour through a console that changes often and has little to do with the project. It also sends the data to a third party.

### Options

| Option | What it is | Fit |
|---|---|---|
| CSV file | Node-RED core `file` node appends a line per reading | Zero extra installs. Opens in LibreOffice or any spreadsheet. Good first step. |
| SQLite | `node-red-node-sqlite` (2.0.1, April 2026); single database file on the Pi | Tested: installs and runs on Node.js 26.12. Teaches basic SQL; data can be queried back into dashboard charts. No server to run. |
| InfluxDB + Grafana | Time-series database and charting server | The "proper" IoT stack and very capable, but two more services to install and secure. The Node-RED node (`node-red-contrib-influxdb`) was last updated in 2023. Better as an optional extension. |
| Baserow or NocoDB | Self-hosted, open-source spreadsheet-style databases with a REST API | Closest to the Google Sheets experience (a browser table). No maintained Node-RED node; write with the core `http request` node. Heavier to self-host on a Pi. |

### Recommendation

Teach logging in two steps:

1. CSV with the core `file` node, to show the idea with no installs.
2. SQLite as the main logger, with a dashboard chart reading history back from the database.

Mention InfluxDB/Grafana and Baserow as further options without building them in the main project. Everything stays on the student's Pi.

The existing "Prepare Sheet Payload" function already builds one comma-separated row from flow variables; it needs only small changes to feed either the CSV or an SQL `INSERT`. Also replace the hand-built date string (which does not zero-pad months, minutes or seconds) with an ISO 8601 timestamp.

## 3. Alerts: replace IFTTT

### Problem

IFTTT Webhooks is now a Pro (paid) service, so the MCU low-voltage alert in flow v6 silently fails on a free account.

### Options

| Option | How it works | Fit |
|---|---|---|
| ntfy | Open-source push notification service. Publish with a plain HTTP POST to a topic URL; phone app receives it. Use the free public server at ntfy.sh or self-host on the Pi. | Smallest change: the flow already uses an `http request` node, so only the URL and headers change. Self-hosting covers "roll your own". |
| Gotify | Open-source, self-hosted push server with an Android app | Similar to ntfy, but self-host only and no official iOS app. |
| Email | `node-red-node-email` (5.2.5, July 2026) over SMTP | No new service, but SMTP providers increasingly require app passwords or OAuth, which makes setup fiddly. |
| Telegram | `node-red-contrib-telegrambot` (19.0.3, September 2026; needs Node.js 22.19 or later) | Free and reliable; needs a Telegram account and a bot token. |

### Recommendation

Use ntfy as the primary alert channel and show both routes: the public ntfy.sh server for a quick start, then self-hosting ntfy on the same Pi so the whole system runs locally. Offer email as an alternative lecture for students who prefer it.

Check before recording: the current free-tier limits on the public ntfy.sh server.

While reworking the alert, improve its logic. The current flow rate-limits to one message per ten minutes and drops the rest, so a low battery generates an alert every ten minutes indefinitely. A latch (alert once when the voltage crosses the threshold, reset when it recovers) is a better pattern to teach. The "Trigger alert?" switch also compares the voltage as a string; make it a numeric comparison.

## 4. Dashboard: decide between Dashboard 1 and 2

`node-red-dashboard` (Dashboard 1) still works on Node-RED 5 but is deprecated. The replacement is FlowFuse Dashboard (`@flowfuse/node-red-dashboard`, Dashboard 2.0). The migration tool converts tabs and groups only; widgets must be rebuilt by hand.

Recommendation: rebuild the dashboard in Dashboard 2.0 for the re-recorded course. Keep the Dashboard 1 flows (v1 to v6) in the repository for students on the old lectures, and add the new flows alongside them rather than overwriting.

## 5. ESP32 firmware

- EspMQTTClient has had no updates since January 2023. It still uses only APIs that exist in arduino-esp32 core 3.x, but consider moving to a maintained MQTT library (PubSubClient directly, or 256dpi's arduino-mqtt) while the sketch is being changed for the new sensor anyway.
- TaskScheduler is actively maintained; keep it.
- Compile and test the final sketch on the current arduino-esp32 core before recording. This has not yet been done for the 2026 notes.

## 6. Course content

Lectures to re-record, in suggested order:

1. Installing Node-RED 5 (new install script, Node.js version choice, Pi model requirements).
2. Tour of the Node-RED 5 editor.
3. Mosquitto 2 setup with a listener and password file.
4. Dashboard (Dashboard 2.0 rebuild).
5. Air sensor wiring and firmware (SHT4x).
6. Data logging (CSV, then SQLite).
7. Alerts (ntfy, public then self-hosted).

The MQTT, function node, flow context and pump control lectures should need little or no change, because that logic still runs unchanged.

## 7. Repository changes when the update ships

- New flow files named for the new version (for example `TerrariumController-Flow-v7.json`), leaving v1 to v6 untouched.
- New sketch folder for the SHT4x firmware.
- Updated KiCad project if the sensor connector changes.
- Update `UPDATE-2026.md` and the README to say which lectures correspond to which files.

## Open decisions

- SHT4x confirmed as the replacement sensor?
- Dashboard 2.0, or UIBUILDER for students who want to learn plain HTML/JS?
- Keep the Raspberry Pi DHT22 examples as a legacy section, or remove them?
- Self-hosted ntfy as the default, or the public server first?
