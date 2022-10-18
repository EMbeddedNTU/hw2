#!/usr/bin/env python3

import socket
import numpy as np
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import threading
# import queue

HOST = '192.168.0.102' # IP address 
PORT = 5555 # Port to listen on (use ports > 1023) 


time_axis = []
ys = []
data = []

class DataThread(threading.Thread):
    def __init__(self, conn: socket, *args, **kwargs):
        super(DataThread, self).__init__(*args, **kwargs)
        self.conn = conn
        self._stop_flag = threading.Event()
 
    def stop(self):
        self._stop_flag.set()
 
    def active(self):
        return not self._stop_flag.isSet()
 
    def run(self):
        global data
        while self.active():
            res = self.conn.recv(1024).decode('utf-8')
            try:
                data.append(json.loads(res))
            except:
                pass
            data = data[-20:]
            print('Received from socket server : ', data[-1])

def animate(i, xs:list, ys:list):
    # Set up x-axis as time axis
    xs.append(time.time()*1000 - start_ms)
    xs = xs[-100:]

    # Draw all data
    k_values = list(data[-1].items())
    for i in range(6):
        (k, v) = k_values[i]
        ys[i].append(v)
        ys[i] = ys[i][-100:]
        # Plot
        ax = axs[i//3, i%3]
        ax.clear()
        ax.set_title(k)
        ax.set_xlabel('time(ms)')
        ax.set_ylabel(k)
        ax.plot(xs, ys[i])


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s: 
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        audit_data_thread = DataThread(conn)
        audit_data_thread.start()

        # Wait for first data to arrive
        while len(data) == 0:
            time.sleep(1.0)

        # Set up animation
        ys = [[] for _ in range(6)]
        fig, axs = plt.subplots(2, 3)
        start_ms = time.time()*1000
        fig.tight_layout(pad=4.0)

        ani = animation.FuncAnimation(fig, animate, fargs=(time_axis, ys), interval=100)
        plt.show()

        audit_data_thread.stop()
        audit_data_thread.join()
        
        conn.close()
        s.close()

        # ax = plt.axes(projection='3d')