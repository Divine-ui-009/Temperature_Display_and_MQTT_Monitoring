# Temperature Display and MQTT Monitoring
Trade Code: SPE - Embedded Systems Software Integration

## 1. Overview
This project reads temperature from a DHT11 sensor using an Arduino Uno,
displays the candidate's name and the temperature on a 16x2 LCD, sends the
reading to a PC over USB serial, and a Python program on the PC publishes
each reading to an MQTT broker hosted on a VPS while showing the values in
real time.

## 2. System Architecture

```
[ Temperature Sensor (LM35 / DHT11) ]
                ↓
        [ Arduino Uno ]
     ┌──────────┼──────────┐
     ↓          ↓          ↓
 [ LCD 16x2 ] [ Serial USB ] (TX/RX)
     ↓                     ↓
 Display Name + Temp   [ PC Program ]
                           ↓
                    [ MQTT Client ]
                           ↓
                    [ MQTT Broker (VPS) ]
                           ↓
                     Dashboard / Subscriber
```

## 3. Hardware Used
- Arduino Uno
- DHT11 temperature sensor
- 16x2 character LCD (parallel, HD44780-compatible)
- Jumper wires
- USB cable (Arduino to PC)

## 4. Wiring

| LCD pin | Arduino pin |
| GND | GND |
| VCC | 5V |
| SDA | A4 |
| SCL | A5 |


| DHT11 pin | Arduino pin |
|---|---|
| VCC | A2 | #no availabe 5v pin
| GND | GND |
| DATA | D2 (add 10k pull-up to 5V if module has no built-in pull-up) |

## 5. Repository Structure
```
arduino/temp_lcd_serial.ino   - Arduino sketch (Part 1)
pc_client/pc_client.py        - PC serial-to-MQTT client (Part 2)
pc_client/requirements.txt    - Python dependencies
docs/architecture.png         - architecture diagram
screenshots/                  - execution proof
```

## 6. Setup and Run

### Arduino side
1. Open `arduino/temp_lcd_serial.ino` in the Arduino IDE.
2. Install the `DHT sensor library` by Adafruit (and its dependency
   `Adafruit Unified Sensor`) via Library Manager.
3. Set `CANDIDATE_NAME` at the top of the file to your name.
4. Select board "Arduino Uno" and the correct COM port, then upload.

### PC side
1. `cd pc_client`
2. `pip install -r requirements.txt`
3. Edit `pc_client.py`:
   - `SERIAL_PORT` to match your Arduino's port (e.g. `COM3` or `/dev/ttyACM0`)
   - `MQTT_BROKER` to your VPS IP/hostname
   - `MQTT_TOPIC` if you want a different topic name
4. Run: `python pc_client.py`

## 7. Communication Names Used

- **Serial communication (Arduino -> PC):** USB serial at **9600 baud**,
  line format `TEMP:<value>` (e.g. `TEMP:24.5`), newline-terminated.
- **MQTT topic used for publishing:** `spe/practical/temperature`
- **MQTT client ID:** `arduino-temp-client`

## 8. Part 1 Details (Arduino)
- Reads temperature from DHT11 every 2 seconds.
- LCD row 1 shows the candidate's name; if longer than 16 characters it
  scrolls horizontally (left to right, looping) using `millis()`-based
  non-blocking timing.
- LCD row 2 shows the current temperature in Celsius.
- Sends each reading over serial as `TEMP:<value>`.

## 9. Part 2 Details (PC client)
- Opens the serial port and reads incoming lines.
- Parses lines matching `TEMP:<value>` with a regex.
- Publishes the numeric value to the configured MQTT topic.
- Prints a timestamped line to the console for every reading received
  (real-time monitoring).

## 10. MQTT Broker / Dashboard
- Broker: Mosquitto running on a VPS (replace with your VPS details).
- To verify publishing manually:
  ```
  mosquitto_sub -h your.vps.ip -t spe/practical/temperature -v
  ```
- Dashboard link: _add your dashboard link here before submission_
  (e.g. a simple HTML/Node-RED/Grafana page subscribed to the same topic).

## 11. Screenshots
See `screenshots/` for:
- LCD showing name (scrolling) and temperature.
- Arduino Serial Monitor showing `TEMP:` lines.
- PC client terminal showing received values and MQTT publish confirmation.
- `mosquitto_sub` or dashboard showing the values arriving on the broker.

## 12. Author
- Candidate Name: IRASUBIZA Divine
- Trade Code: SPE (Embedded Systems Software Integration)
