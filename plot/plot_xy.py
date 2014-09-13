#!/usr/bin/python -Wall

# Quick hack for plotting csv files of points in the xy-plane.
# Requires matplotlib
# Usage: python plot_xy.py [filename]
#
# Comments in csv file should be preceded by '#'
#
# First column in the file should be the x-coordinate,
# second column should be the y-coordinate , i.e.
#   1.62,3.24
#   2.72,5.44
#   3.14,6.28
# ...etc.

import sys, pylab

# I/O routine
argc = len(sys.argv)
if argc < 2:
    print >> sys.stderr, "Usage: %s [file1] [file2] ..." % sys.argv[0]
    sys.exit(1)

for i in range(1, argc):
    x_list = []
    y_list = []

    file = open(sys.argv[i], "r")

    # get list of x, y coordinates
    for line in file:
      point = line.rstrip().split(',')
      if(point[0][0] != '#'):
          try:
              float(point[0])
              float(point[1])
          except:
              print >> sys.stderr, \
                ("Bad input: '%s' must contain columns of numbers separated "
                 "by commas.\nComments must be preceded by a '#' symbol.") \
                % sys.argv[i]
              sys.exit(1)
          x_list.append(float(point[0]))
          y_list.append(float(point[1]))

# plot
if(i % 9 == 0):
    pylab.plot(x_list, y_list,'g')
elif(i % 9 == 1):
    pylab.plot(x_list, y_list,'b')
elif(i % 9 == 2):
    pylab.plot(x_list, y_list,'r')
elif(i % 9 == 3):
    pylab.plot(x_list, y_list,'c')
elif(i % 9 == 4):
    pylab.plot(x_list, y_list,'m')
elif(i % 9 == 5):
    pylab.plot(x_list, y_list,'y')
elif(i % 8 == 6):
    pylab.plot(x_list, y_list,'k')
elif(i % 9 == 7):
    pylab.plot(x_list, y_list,'w')
else:
    pylab.plot(x_list, y_list,'r')

pylab.show()

file.close
