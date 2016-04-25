#!/bin/bash

FILE=./dict.dat

cd data

if [ -f $FILE ]; then

gnuplot -persist <<-EOFMarker

set size 1,1
set origin 0,0
set multiplot

set grid
set xtics 0.2
set ytics 0.1

# First graph
set size 0.5,0.72
set origin 0,0.28
set xlabel "[u1]"
set ylabel "[u2]"
set key out vert bot center

plot 'dict.dat' index 0 using 1:2 with points pt 6 ps 2 lc rgb 'red' title "Carcinoid",\
'dict.dat' index 1 using 1:2 with points pt 4 ps 2 lc rgb 'green' title "Colon",\
'dict.dat' index 2 using 1:2 with points pt 2 ps 2 lc rgb 'blue' title "Normal",\
'dict.dat' index 3 using 1:2 with points pt 8 ps 2 lc rgb 'black' title "SmallCell"


# Second graph
set size 0.5,0.5
set origin 0.5,0.5
set xlabel "[u1]"
set ylabel "[u3]"
set nokey

plot "dict.dat" index 0 using 1:3 with points pt 6 ps 2 lc rgb "red",\
"dict.dat" index 1 using 1:3 with points pt 4 ps 2 lc rgb "green",\
"dict.dat" index 2 using 1:3 with points pt 2 ps 2 lc rgb "blue",\
"dict.dat" index 3 using 1:3 with points pt 8 ps 2 lc rgb "black"


# Third graph
set size 0.5,0.5
set origin 0.5,0
set xlabel "[u2]"
set ylabel "[u3]"
set nokey

plot "dict.dat" index 0 using 2:3 with points pt 6 ps 2 lc rgb "red",\
"dict.dat" index 1 using 2:3 with points pt 4 ps 2 lc rgb "green",\
"dict.dat" index 2 using 2:3 with points pt 2 ps 2 lc rgb "blue",\
"dict.dat" index 3 using 2:3 with points pt 8 ps 2 lc rgb "black"


unset multiplot
reset

EOFMarker

else
echo "Error: Data file does not exist."

fi