import serial
import csv
from datetime import datetime

arduino_port = 'COM3'  
baud_rate = 9600

ser = serial.Serial(arduino_port, baud_rate, timeout=1)

with open('arduino_data.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['Date & Time', 'Temperature(C)', 'Humidity(%)'])
    try:
        while True:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print(line)
                fields = line.split(',')
                if len(fields) == 2:
                    now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                    writer.writerow([now] + fields)
                    csvfile.flush()
    except KeyboardInterrupt:
        print("Logging stopped by user.")

ser.close()
