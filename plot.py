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
    air_p_data = deque(maxlen=MAX_PTS)

    app = QtWidgets.QApplication([])

    win = pg.GraphicsLayoutWidget(show=True)
    win.setWindowTitle("Blood Pressure and Air Pressure vs Time")

    pressure_plot = win.addPlot(row=0, col=0)
    pressure_plot.setLabel('left', 'Pressure', units='mmHg')
    pressure_plot.setLabel('bottom', 'Time', units='s')
    pressure_plot.showGrid(x=True, y=True)

    pressure_curve = pressure_plot.plot()

    win.nextRow()

    air_pressure_plot = win.addPlot(row=0, col=1)
    air_pressure_plot.setLabel('left', 'Air Pressure', units='mmHg')
    air_pressure_plot.setLabel('bottom', 'Time', units='s')
    air_pressure_plot.showGrid(x=True, y=True)

    air_pressure_curve = air_pressure_plot.plot()

    def update():
        while True:
            if not ser.in_waiting:
                break

            try:
                line = ser.readline().decode('utf-8').strip()
                if not line:
                    continue

                t, p, air_p = line.split(',')

                t_data.append(float(t))
                p_data.append(float(p))
                air_p_data.append(float(air_p))

            except (ValueError, UnicodeDecodeError):
                pass

        if len(t_data) > 1:
            t = np.asarray(t_data)

            pressure_curve.setData(t, np.asarray(p_data))
            air_pressure_curve.setData(t, np.asarray(air_p_data))

            if len(t_data) % 20 == 0:
                xmin = max(0, t_data[-1] - 10)
                xmax = t_data[-1] + 1

                pressure_plot.setXRange(xmin, xmax)
                air_pressure_plot.setXRange(xmin, xmax)

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
