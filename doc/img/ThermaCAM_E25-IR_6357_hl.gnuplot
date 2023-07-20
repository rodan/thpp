#!/usr/bin/env gnuplot

plot 'ThermaCAM_E25-IR_6357_hl.csv' with lines

set ylabel '°C' rotate by 0
set terminal png size 1024,300
set output 'ThermaCAM_E25-IR_6357_hl_gnuplot.png'
set style data lines
replot
