#!/usr/bin/env gnuplot

plot 'FLIR_P60_hl_line.csv' title "" with lines lw 2

set grid y
set grid lc rgb "#dddddd" lt 1
unset xtics
set ylabel '°C' rotate by 0
set terminal png size 1024,300
set output 'FLIR_P60_hl_line_gnuplot.png'
set style data lines
replot
