# -*- coding: utf-8 -*-
"""
Created on Mon Mar 23 22:00:01 2020

@author: alufi
"""

from math import *
#k es delta z, a es delta theta, z es z minimo
def L(k,a,z):
    return (k + z) / (2 * k) * sqrt(a**2 * k**2 + 2 * a**2 * k * z + a**2 * z**2 + k**2) + k/(2 * a) * log(a * (sqrt(a**2 * k**2 + 2 * a**2 * k * z + a**2 * z**2 + k**2) + a * k + a * z)) - z / (2 * k) * sqrt(a**2 * z**2 + k**2) - k/(2 * a) * log(a * (sqrt(a**2 * z**2 + k**2) + a * z))

print(L(30, 125.6637061, 0))

"""(k + z) / (2 * k) * \sqrt{a^2 * k^2 + 2 * a^2 * k * z + a^2 * z^2 + k^2} + k/(2 * a) * \log_e{a * (sqrt(a^2 * k^2 + 2 * a^2 * k * z + a^2 * z^2 + k^2) + a * k + a * z)} - z / (2 * k) * \sqrt{a^2 * z^2 + k^2} - k/(2 * a) * \log_e{a * (\sqrt{a^2 * z^2 + k^2} + a * z)}"""

import csv
import numpy as np
import matplotlib.pyplot as plt

def drawThr():
    with open("../Archivos Thr/THR e imagenes/dougs3.thr") as csv_file:
        csv_reader = csv.reader(csv_file)
        onlyValidLines = []
        for line in csv_reader:
            
            if (len(line) > 0):
                line[0] = line[0].replace('\t', ' ')
                lineStr = line[0].replace(' ', '', line[0].count(' ') - 1)
                lineList = lineStr.split(' ')
                if (len(lineList) > 1):
                    if (isfloat(lineList[0]) and isfloat(lineList[1])):
                        onlyValidLines.append(lineList)
        try:
            theta, z = zip(*onlyValidLines)
        except ValueError:
            print("Error en: {}".format(name))
            return
        
    theta = np.array(theta, dtype = 'double')
    z = np.array(z, dtype = 'double')
    
    x = z * np.cos(theta)
    y = z * np.sin(theta)
    
    #plt.plot(x[:], y[:], '.-', markersize = 0.01, color = 'y', linewidth=0.5)
    plt.plot(x[:], y[:], '.', markersize = 0.2, color = 'm')
    plt.show()

def drawInterpolate(ti, zi, tf, zf, slices):
    deltaZ = (zf - zi)/slices
    deltaT = (tf - ti)/slices
    theta = []
    z = []
    for i in range(slices):
        theta.append(ti + deltaT*i)
        z.append(zi + deltaZ*i)
    
    theta = np.array(theta, dtype = 'double')
    z = np.array(z, dtype = 'double')
    x = z * np.cos(theta)
    y = z * np.sin(theta)
    plt.plot(x[:], y[:], '*-')
    plt.show()
    
#drawInterpolate(0, 0, 125.66370614, 30, 30)
drawThr()