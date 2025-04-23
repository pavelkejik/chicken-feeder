# Smart Chicken Feeder
_An ESP32-powered accessory for automated feed control_

## Overview
This smart feeder is part of the Automated Chicken Coop Ecosystem. It operates wirelessly and autonomously to dispense food based on schedule, user commands, or weight target.

Powered by a battery-friendly ESP32-C3 and connected via **ESP-NOW** to the main coop door (gateway), the feeder ensures reliable long-term operation and seamless integration into the larger coop management system.

---

## Features
- **Scheduled feeding** with daily times and portion sizes
- **Feed-to-weight** mode using integrated load-cell
- **ESP-NOW** communication for ultra-low power use
- Battery-powered, optimized for deep sleep
- OTA firmware updates via the coop door gateway
- Status reported via MQTT (via gateway) and visible in Web UI

---

## Hardware
| Component | Purpose |
|----------|---------|
| ESP32-C3 | Low-power microcontroller with Wi-Fi and ESP-NOW |
| Load Cell + HX711 | Measure feed weight in grams |
| Servo Motor | Open/close feed flap |
| 18650 Li-Ion + Charger | Power supply for standalone use |

All components are enclosed in a 3D-printable case. STL files and PCB design are in the `hw/` directory.

---

## Communication
### ESP-NOW
- Feeder registers with the door (gateway)

### MQTT (via Gateway)
```text
<base_topic>/devices/<feeder-id>/state/weight_g        = 175
<base_topic>/devices/<feeder-id>/set/feed_g            ← 50
<base_topic>/devices/<feeder-id>/set/mode              ← "schedule" | "weight"
```

---

## Power Management
- Sleeps between operations using deep sleep
- Wakeup on timer or servo/feed command
- Runtime per charge depends on feeding frequency

---

## OTA Updates
- Triggered remotely via the coop door
- Uses ESP-NOW for wire-free, zero-setup OTA

---

## Getting Started
1. Flash firmware using PlatformIO:
   ```bash
   cd firmware
   pio run -t upload
   ```
2. Power up the feeder and press the pairing button once.
3. In the Web UI (at `dvirka.local`), go to **Accessories > New > Pair**.
4. Once connected, it will appear in the dashboard.

---

## Author
**Pavel Kejík**  
Part of the Automated Coop Ecosystem – see main project: [chicken-door](https://github.com/pavelkejik/chicken-door)

