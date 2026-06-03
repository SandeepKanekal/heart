import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
from collections import deque

print('Please close the serial monitor/plotter if open.')

def find_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'CH340' in port.description:
            return port.name
    return None

SERIAL_PORT = find_port()
if not SERIAL_PORT:
    raise ValueError('Port not found')
BAUD_RATE = 9600
MAX_PTS = 100

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

t_data = deque(maxlen=MAX_PTS)
p_data = deque(maxlen=MAX_PTS)

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot([], [])

ax.set_title("Blood pressure against time")
ax.set_xlabel("Time (ms)")
ax.set_ylabel("Pressure (mmHg)")
ax.grid(True)
ax.set_ylim(60, 140)

try:
    while True:
        if ser.in_waiting > 0:
            raw_data = ser.readline()
            if raw_data:
                try:
                    data = raw_data.decode('utf-8').split(',')
                    print(data)

                    t_data.append(float(data[0]))
                    p_data.append(float(data[1]))

                    line.set_xdata(t_data)
                    line.set_ydata(p_data)
                    
                    ax.set_xlim(max(0, t_data[0])-100, t_data[-1]+100)

                    fig.canvas.draw()
                    fig.canvas.flush_events()
                except ValueError:
                    continue
except KeyboardInterrupt:
    print('\n Stopping data collection.')
finally:
    ser.close()
    print('Serial port connection closed.')
        