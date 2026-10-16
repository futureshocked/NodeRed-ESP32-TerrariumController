# Course update notes: Node-RED 5 and Node.js 24/26 (October 2026)

The videos in this course were recorded in mid-2020 with Node-RED 1.1 and Node.js 12. Today's current versions are Node-RED 5.0.x and Node.js 24 (LTS) or 26. This document lists what changed, what still works as shown, and what you need to do differently.

## Short version

The project code still works. The Terrarium Controller flow (`TerrariumController-Flow-v6.json`) was tested in October 2026 on Node-RED 5.0.8 with both Node.js 22.22 and Node.js 26.12, using the original dashboard (`node-red-dashboard` 3.6.6) and a simulated ESP32 over MQTT. The flow loaded, the dashboard served at `/ui`, and the pump control logic published the correct on/off messages. No changes to the flows or the ESP32 sketches were needed.

What has changed is how Node-RED looks and how you install it. The rest of this document covers those differences.

## Tested versions

| Component | In the videos (2020) | Tested October 2026 |
|---|---|---|
| Node-RED | 1.1.x | 5.0.8 |
| Node.js | 12 | 22.22, 26.12 |
| Dashboard | node-red-dashboard 2.x | node-red-dashboard 3.6.6 (deprecated, still works) |
| Google Sheets node | node-red-contrib-google-sheets 1.x | 1.1.2 (loads; not tested against a live sheet) |

Not tested: the ESP32 sketches were not compiled against arduino-esp32 core 3.x for this update, and the Raspberry Pi DHT22 node was not tested on hardware. See the notes below.

## Installing Node-RED

The install command shown in the videos has been replaced. On a Raspberry Pi or any Debian-based Linux, use:

```
bash <(curl -sL \
  https://github.com/node-red/linux-installers/releases/latest/download/install-update-nodered-deb)
```

Add `--node24` or `--node26` at the end to choose the Node.js version. Add `--help` to see all options. Install `build-essential`, `git` and `curl` first (`sudo apt install build-essential git curl`) so that nodes with native code can compile.

Which Node.js version? Node-RED 5 requires Node.js 22.9 or later and its developers recommend Node.js 24. Node.js 26 also works (tested). Node.js 20 and older will not run Node-RED 5.

Raspberry Pi Zero (original) and Pi 1 use the armv6 processor. Node.js 24 has no build for armv6, so these boards are limited to older versions. Use a Pi 3, 4 or 5 with a 64-bit OS for this course.

On Windows and macOS, install Node.js 24 LTS from nodejs.org, then run `npm install -g --unsafe-perm node-red` and start it with `node-red`.

## The editor looks different

Node-RED 5 redesigned the editor. Nodes, wires, the deploy button and the debug panel all work the same way, but the layout and colours are different from the videos:

- The default theme may be dark. To get the light look used in the videos, open the main menu, User Settings, and under Appearance set Theme to Light.
- Sidebars can now be dragged and split, and there is an optional left sidebar.
- The deploy menu has been redesigned. Full, Modified Flows and Modified Nodes deploy options are still there.
- Nodes in the palette are sorted alphabetically.
- The "rbe" node used in the flows now appears in the palette as "filter". It is the same node; imported flows still work.

## The dashboard

The flows in this course use `node-red-dashboard`, the original dashboard (often called Dashboard 1). It was formally deprecated in 2024 and no longer receives updates. It still installs from the Palette Manager and still works on Node-RED 5, which is why the course flows run unchanged. The Palette Manager will show a deprecation warning; you can install it anyway.

Install it from the Palette Manager (search for `node-red-dashboard`) before importing the course flows, otherwise the dashboard nodes show as "unknown".

The replacement is FlowFuse Dashboard (`@flowfuse/node-red-dashboard`, also called Dashboard 2.0). It uses different nodes with different configuration, so the course flows cannot simply be switched over. Stay with Dashboard 1 while you follow the course. A Dashboard 2.0 version of the project is planned for the re-recorded lectures.

## MQTT broker (Mosquitto)

Mosquitto 2.x (the version you get from `apt` on current Raspberry Pi OS) only accepts connections from the same machine unless you configure a listener. If Node-RED connects but the ESP32 cannot, add these lines to a file in `/etc/mosquitto/conf.d/` (for example `local.conf`) and restart Mosquitto:

```
listener 1883
allow_anonymous false
password_file /etc/mosquitto/passwd
```

Create the password file with `sudo mosquitto_passwd -c /etc/mosquitto/passwd <username>` and use the same username and password in `esp32_secrets.h` and in the Node-RED MQTT broker node.

## ESP32 sketches

The sketches use three libraries: EspMQTTClient, TaskScheduler and DHTesp. TaskScheduler is actively maintained. EspMQTTClient has had no updates since 2023 and DHTesp is marked by its author as no longer maintained, but neither uses any of the ESP32 APIs that changed in arduino-esp32 core 3.x. They are expected to compile; this has not yet been confirmed on hardware for this update. If you get a compile error, post it in the Q&A with your ESP32 core version (Tools, Board, Boards Manager, esp32).

## Raspberry Pi DHT22 examples

The early DHT22 examples (`DHT22-ex1-flow.json`, `DHT22-ex2-flow.json`, `DHT22-complete-example-flow.json`) read the sensor directly from the Pi using `node-red-contrib-dht-sensor`. This node compiles native code.

- Pi 3 and Pi 4: install the BCM2835 C library first (see the `node-dht-sensor` documentation), then install the node from the Palette Manager.
- Pi 5: the BCM2835 library does not work, because the Pi 5 uses a different GPIO chip. The underlying `node-dht-sensor` library supports the Pi 5 through libgpiod: `sudo apt install libgpiod-dev pkg-config`, then build it with `npm install node-dht-sensor --use_libgpiod=true` in your `~/.node-red` directory. This has not been tested with the Node-RED wrapper node.

The main Terrarium Controller reads the DHT22 on the ESP32, so it is not affected.

## Third-party services

- **IFTTT** (`ifttt-flow.json` and the MCU voltage alert in flow v6): the Webhooks service now requires an IFTTT Pro subscription. On a free account the alert will not trigger. Any service that accepts an HTTP POST, or Node-RED's own email node (`node-red-node-email`), can replace it.
- **Google Sheets** (flows v3 to v6): `node-red-contrib-google-sheets` has not been updated since 2022 but still installs and loads. It needs a Google Cloud service account JSON key; the Google Cloud console looks different from the videos, but the steps (create project, enable the Sheets API, create service account, download key, share the sheet with the service account email) are the same.
- **dweet.io** (`dweetio-flow.json`): availability of this free service could not be confirmed. If requests fail, skip this example; the course project does not depend on it.

## Credentials

Node-RED 5 stores new credentials files next to your flows file and warns if you have not set your own `credentialSecret` in `settings.js`. Set one (any long string) so your MQTT and Google credentials survive a reinstall.

## Getting help

If something does not work, post in the Q&A with: the lecture number, your Node-RED version (shown in the main menu), your Node.js version (`node -v`), and the exact error from the debug panel or the Node-RED log (`node-red-log` on a Pi).
