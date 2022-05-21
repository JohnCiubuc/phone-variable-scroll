#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat May 21 00:16:45 2022

@author: inathero
"""

import subprocess
import uinput
import numpy as np
import time
from threading import Thread

 
class wheelSender(Thread):
    _wheelDevice = uinput.Device([uinput.REL_WHEEL])
    _delay = -1
    _delayMasterCounter = 0
    _direction = 0
    def __init__(self):
        Thread.__init__(self)
        self.daemon = True
        self.start()
    def addDelay(self, delay):
        if delay > 0:
            self._direction = 1 
        else:
            self._direction = -1
        delay = abs(delay)
        if self._delayMasterCounter != delay:
            self._delayMasterCounter = delay
            self._delay = delay
    def run(self):
        while True:
            # Fastest sleep speed
            self._delay = self._delay - 1
            if self._delay == 0:
                print(self._direction)
                self._wheelDevice.emit(uinput.REL_WHEEL, 1) if self._direction >=0 \
                    else self._wheelDevice.emit(uinput.REL_WHEEL, -1)
                self._delay = self._delayMasterCounter
            
            if self._delay <= -1:
                self._delay = -1
            time.sleep(0.01)

def getScreenSize():
    p1=subprocess.Popen(r'adb shell wm size',shell=True,stdout=subprocess.PIPE)
    for line in p1.stdout:
        line = line.decode(encoding="utf-8", errors="ignore")
        line = line.strip().split(' ')
        if len(line) == 3:
            diff = line[2].split('x')
            size = ()
            try:
                size = (int(diff[0]), int(diff[1]))
            except Exception as e:
                print(e)
            return size





screenDivisions = 24
screenDivisionsHalf = screenDivisions / 2

screen = getScreenSize()
if screen == None:
    print("warning: Can't get screensize. Is ADB running?")
    raise SystemExit
screenSpaces = np.linspace(0,screen[1],screenDivisions)
wheeler = wheelSender()
try:
    p1=subprocess.Popen(r'adb shell getevent',shell=True,stdout=subprocess.PIPE)
    for line in p1.stdout:
        line = line.decode(encoding="utf-8", errors="ignore")
        line = line.strip().split(' ')
        if line[2] == '0036':
            h = int(line[3], 16)
            index = list(filter(lambda e: e>= h, screenSpaces))[0] #maybe dangerous? but last item shoul be fine
            index = np.where(screenSpaces==index)
            index = screenDivisionsHalf - index[0][0]
            
            if index > 0:
                index = screenDivisionsHalf - index
            elif index < 0:
                index = -1 * (screenDivisionsHalf + index)
            
            print(index)
            wheeler.addDelay(index)
except Exception as e:
    print(e)