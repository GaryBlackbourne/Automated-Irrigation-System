cd "/home/geri/dev/test"

data = "data.log"

set xdata time
set timefmt "%d/%m/%Y %H:%M:%S"
set format x "%d/%m/%Y %H:%M:%S"

set yrange [0:40000]
set terminal qt 0
plot data using 1:4 with lines

set yrange [0:10000]
set terminal qt 1
plot data using 1:6 with lines

set yrange [0:40]
set terminal qt 2
plot data using 1:8 with lines
