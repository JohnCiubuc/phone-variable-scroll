#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat May 21 00:16:45 2022

@author: inathero
"""

import subprocess
import uinput

def get_xy():
    cmd = r'adb shell getevent'
    w = 0
    h = 0
    try:
        p1=subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
        for line in p1.stdout:
            line = line.decode(encoding="utf-8", errors="ignore")
            line = line.strip().split(' ')
            if line[2] == '0035':
                w = int(line[3], 16)
            elif line[2] == '0036':
                h = int(line[3], 16)
                if h >0:
                    p = (w, h)
                    print(p) 
            # if  ' 0036 ' in line:
            #     e = line.split(" ")
            #     h = int(e[3], 16)
            #     if h >0:
            #         p = (w, h)
            #         print(p) 
        p1.wait()
        
    except Exception as e:
        print(e)
# get_xy()
# print(size)


import time
device = uinput.Device([uinput.REL_WHEEL])

# time.sleep(2);
 
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
# for i in range(20):
#     time.sleep(0.2);
#     device.emit(uinput.REL_WHEEL, 1)
# print('done')

print(getScreenSize())