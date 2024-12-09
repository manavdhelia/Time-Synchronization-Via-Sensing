# Python code for Raspberry Pi to communicate with two ESP32 devices via serial,
# calculate the timestamp offset, and send corrected time back to the ESP32 devices.

import serial
import time

# Serial ports for ESP32 devices
ESP1_PORT = '/dev/ttyUSB0'  # Update this to your ESP32 port
ESP2_PORT = '/dev/ttyUSB1'  # Update this to your ESP32 port
BAUDRATE = 115200

# Initialize serial connections
esp1 = serial.Serial(ESP1_PORT, BAUDRATE, timeout=1)
esp2 = serial.Serial(ESP2_PORT, BAUDRATE, timeout=1)

# Function to parse timestamp from incoming data
def parse_timestamp(data):
    try:
        if data.startswith("TIMESTAMP:"):
            return float(data.split(":")[1])
    except Exception as e:
        print(f"Error parsing timestamp: {e}")
    return None

# Function to send corrected time to ESP32
def send_correction(serial_conn, offset):
    try:
        correction_msg = f"CORRECTION:{offset}\n"
        serial_conn.write(correction_msg.encode())
    except Exception as e:
        print(f"Error sending correction: {e}")

print("Raspberry Pi Time Synchronization Program Started")

try:
    while True:
        # Read data from ESP32 devices
        esp1_data = esp1.readline().decode().strip()
        esp2_data = esp2.readline().decode().strip()

        # Parse timestamps
        esp1_timestamp = parse_timestamp(esp1_data)
        esp2_timestamp = parse_timestamp(esp2_data)

        # If both timestamps are received, calculate the offset
        if esp1_timestamp is not None and esp2_timestamp is not None:
            print(f"ESP1 Timestamp: {esp1_timestamp}, ESP2 Timestamp: {esp2_timestamp}")

            # Calculate offset
            offset = esp2_timestamp - esp1_timestamp

            # Send corrected time to both ESP32 devices
            send_correction(esp1, offset)  # Adjust ESP1 to match ESP2
            # No correction sent to ESP2

            print(f"Offset: {offset} sent to ESP1, {-offset} sent to ESP2")

        time.sleep(0.1)

except KeyboardInterrupt:
    print("Exiting program...")

finally:
    esp1.close()
    esp2.close()
    print("Serial connections closed.")
