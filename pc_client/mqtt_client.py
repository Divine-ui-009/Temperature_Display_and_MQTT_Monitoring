"""
Temperature Display and MQTT Monitoring - Part 2
Trade Code: SPE (Embedded Systems Software Integration)

Reads "TEMP:<value>" lines from the Arduino over USB serial, publishes each
value to an MQTT broker, and prints incoming readings to the console in
real time.

Install dependencies:
    pip install pyserial paho-mqtt

Run:
    python pc_client.py
"""

import re
import sys
import time
from datetime import datetime

import serial
import paho.mqtt.client as mqtt

# ----- USER CONFIG -----
SERIAL_PORT = "COM7"          # Windows e.g. "COM3" | Linux/Mac e.g. "/dev/ttyACM0" or "/dev/ttyUSB0"
BAUD_RATE = 9600

MQTT_BROKER = "broker.benax.rw"           # <-- VPS broker address
MQTT_PORT = 1883
MQTT_TOPIC = "sensor_irasubiza_divine"    # communication name used for publishing
MQTT_CLIENT_ID = "arduino-temp-client"
MQTT_USERNAME = None    # set if your broker requires auth, else leave None
MQTT_PASSWORD = None
# ------------------------

TEMP_PATTERN = re.compile(r"TEMP:(-?\d+\.?\d*)")


def on_connect(client, userdata, flags, reason_code, properties=None):
    if reason_code == 0:
        print(f"[MQTT] Connected to broker {MQTT_BROKER}:{MQTT_PORT}")
    else:
        print(f"[MQTT] Connection failed, reason code: {reason_code}")


def on_publish(client, userdata, mid, reason_code=None, properties=None):
    pass  # keep console clean; main loop already prints each reading


def build_mqtt_client():
    client = mqtt.Client(client_id=MQTT_CLIENT_ID, callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
    if MQTT_USERNAME:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_connect = on_connect
    client.on_publish = on_publish
    client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
    client.loop_start()
    return client


def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"[SERIAL] Listening on {SERIAL_PORT} @ {BAUD_RATE} baud")
    except serial.SerialException as e:
        print(f"[SERIAL] Could not open {SERIAL_PORT}: {e}")
        sys.exit(1)

    client = build_mqtt_client()

    time.sleep(2)  # allow Arduino to reset after serial connection opens

    print(f"[MQTT] Publishing to topic: {MQTT_TOPIC}")
    print("Listening for temperature data... (Ctrl+C to stop)\n")

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            match = TEMP_PATTERN.search(line)
            if match:
                temp_value = match.group(1)
                timestamp = datetime.now().strftime("%H:%M:%S")

                # Real-time display
                print(f"[{timestamp}] Temperature: {temp_value} C")

                # Publish to MQTT broker
                client.publish(MQTT_TOPIC, payload=temp_value, qos=0, retain=False)
            elif line == "ERR:SENSOR":
                print(f"[{datetime.now().strftime('%H:%M:%S')}] Sensor read error on Arduino")
            else:
                print(f"[RAW] {line}")

    except KeyboardInterrupt:
        print("\nStopping client...")
    finally:
        client.loop_stop()
        client.disconnect()
        ser.close()
        print("Serial port closed, MQTT disconnected.")


if __name__ == "__main__":
    main()