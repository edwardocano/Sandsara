# -*- coding: utf-8 -*-
"""
Created on Mon Mar 23 22:00:01 2020

@author: alufi
"""

from math import *
import csv
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import os 
from os import listdir
from os.path import isfile, join

mpl.rcParams['figure.dpi'] = 108

dirPath = os.getcwd()
dirPath = dir_path.replace("\\" , "/")

print (dir_path)

x = []
y = []

def isfloat(value):
    try:
        float(value)
        return True
    except ValueError:
        return False

           
def drawThr(name):
    with open("../Archivos Thr/THR e imagenes/" + name) as csv_file:
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
    
    plt.figure(figsize=(10,10))
    theta = np.array(theta, dtype = 'double')
    z = np.array(z, dtype = 'double')
    x = []
    y = []
    for i in range(1, theta.size):
        factor = 0;
        factor1 = z[i] - z[i-1]
        factor2 = theta[i] - theta[i-1]
        #print (factor2)
        
        factor1 = 1000 * factor1;
        factor2 = 1000 / (2*pi) * factor2;
        
        if (abs(factor1) > abs(factor2)):
            factor = factor1;
        else:
            factor = factor2;
            
        factor = abs(int(factor))
        #if(i < 100):
        #    print(factor)
        
        if (factor > 0):
            zD = (z[i] - z[i-1]) / factor
            tD = (theta[i] - theta[i-1]) / factor
            #print("zD: {} tD: {}".format(zD,tD))
            
            x.append(z[i-1] * np.cos(theta[i-1]))
            y.append(z[i-1] * np.sin(theta[i-1]))
            #print ("x {} y {}".format(x[0], y[0]))
            
            for j in range(1, factor):
                x.append( (z[i-1] + zD*j) * np.cos(theta[i-1] + tD*j) )
                y.append( (z[i-1] + zD*j) * np.sin(theta[i-1] + tD*j) )
                
    plt.plot(x, y, '-', color = 'y', linewidth=0.5)
    plt.savefig("outputImage/" + name[0:name.find(".")] + ".png")
    plt.close();

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
#drawThr("WiltonWindmill 5-22-2010.thr")

onlyfiles = [f for f in listdir("../Archivos Thr/THR e imagenes") if isfile(join("../Archivos Thr/THR e imagenes", f))]

thrFiles = []
for fileName in onlyfiles:
    if (fileName.find(".thr") > 0):
        thrFiles.append(fileName)
    
for name in thrFiles:
    drawThr(name)















