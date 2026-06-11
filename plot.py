import serial
import signal
import numpy as np
import serial.tools.list_ports
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtWidgets
from collections import deque

print('Please close the serial monitor/plotter if open.')

def find_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'CH340' in port.description:
            return port.device
    return None

SERIAL_PORT = find_port()
if not SERIAL_PORT:
    raise RuntimeError('Serial port could not be found')

BAUD_RATE = 115200
MAX_PTS = 1000

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
except serial.serialutil.SerialException:
    print('Serial port being accessed elsewhere.')
else:
    t_data = deque(maxlen=MAX_PTS)
    p_data = deque(maxlen=MAX_PTS)

    app = QtWidgets.QApplication([])

    plot = pg.PlotWidget()
    plot.show()
    plot.setWindowTitle("Blood Pressure vs Time")
    plot.setLabel('left', 'Pressure', units='mmHg')
    plot.setLabel('bottom', 'Time', units='s')
    plot.showGrid(x=True, y=True)

    curve = plot.plot()

    def update():
        while True:
            if not ser.in_waiting:
                break

            try:
                line = ser.readline().decode('utf-8').strip()
                if not line:
                    continue

                t, p = line.split(',')

                t_data.append(float(t))
                p_data.append(float(p))

            except (ValueError, UnicodeDecodeError):
                pass

        if len(t_data) > 1:
            curve.setData(np.asarray(t_data), np.asarray(p_data))

            if len(t_data) % 20 == 0:
                plot.setXRange(max(0, t_data[-1] - 10), t_data[-1] + 1)

    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(20)

    def cleanup():
        if ser.is_open:
            ser.close()
            print("Serial port connection closed.")

    app.aboutToQuit.connect(cleanup)

    signal.signal(signal.SIGINT, signal.SIG_DFL)

    quit_timer = QtCore.QTimer()
    quit_timer.timeout.connect(lambda: None)
    quit_timer.start(100)

    app.exec()
