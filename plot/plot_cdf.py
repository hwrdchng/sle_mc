#!/usr/bin/python -Wall

# Quick hack for plotting cdfs of random variables written to a file in a single
# column
# Requires numpy, matplotlib
# Usage: python plot_cdf.py [filename]
#
# Comments in csv file should be preceded by '#'


import sys, math, pylab, numpy

#======Parameters=============================================================
# divide the range of the random variable into num_bin
# equally spaced intervals
num_bin = 20

# expected cdf, in the example below we use 1-(1-x^2)^(5/8)
# which is the expected cdf for horizontal distance ratio
# for SLE curves
expected = lambda x: 1 - pow(1 - x * x, 5.0/8.0)
#============================================================================

# I/O routine
argc = len(sys.argv)
if argc != 2:
  print >> sys.stderr, "Usage: %s [filename].csv" % sys.argv[0]
  sys.exit(1)
file = open(sys.argv[1], "r")
bin_i = 1

# populate list of random variables
rv_list = []
for line in file:
  rv_sample = line.rstrip()

  if(rv_sample[0] != '#'):
    try:
      float(rv_sample)
    except:
      print >> sys.stderr, \
      ("Bad input: '%s' must contain a single column of random variables.\nComments must be preceded by a '#' symbol.") \
      % sys.argv[1]
      sys.exit(1)
    rv_list.append(float(rv_sample))

if len(rv_list) < 2:
 print ("Not enough samples")
 sys.exit(1)

# compute min, max, bin_sizes
rv_list.sort()
min_rv = rv_list[0]
max_rv = rv_list[len(rv_list) - 1]
bin_size = (max_rv - min_rv)/num_bin

# initialize x, y lists for cdf plot
#   x[i] is the upper bound of bin i
#   y[i] is the percent of random variables with values
#   less than or equal to x[i]
# the 0.5 * bin_size is necessary to minimize the effect of
# floating point errors
x_list = numpy.arange(min_rv, max_rv + 0.5 * bin_size, bin_size)
if len(x_list) != num_bin + 1:
  print "Warning: floating point errors disrupting bins"
y_list = [0] * len(x_list)

# compute number of random variables in each bin
bin_i = 1
for j in range(0, len(rv_list)):
  if bin_i == len(x_list) - 1: # needed to handle float
    y_list[bin_i] = len(rv_list)
    break
  elif rv_list[j] <= x_list[bin_i]:
    y_list[bin_i] = y_list[bin_i] + 1
  else:
    while rv_list[j] > x_list[bin_i]:
      bin_i = bin_i + 1
      y_list[bin_i] = y_list[bin_i - 1]
    y_list[bin_i] = y_list[bin_i - 1] + 1

# normalize
y_list[:] = [y * 1.0/len(rv_list) for y in y_list]

# theoretical cdf for rv
x_l = math.floor(min_rv)
x_r = math.ceil(min_rv)
x_expect = numpy.arange(x_l, x_r + (x_r-x_l)/200.0, (x_r - x_l)/100.0)
y_expect = expected(x_expect)
pylab.plot(x_expect, y_expect, 'b')

# empirical cdf for rv
pylab.plot(x_list, y_list, 'ro')

pylab.show()

file.close
