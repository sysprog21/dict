reset                                                                           
set ylabel 'time(sec)'
set style fill solid
set key center top 
set title 'perfomance comparison'
set term png enhanced font 'Verdana,10'
set output 'runtime.png'

plot [:][y=0:0.250]'output.txt' using 2:xtic(1) with histogram title 'cpy', \
'' using 3:xtic(1) with histogram title 'ref' , \
'' using ($0-0.500):(0.110):2 with labels title ' ' textcolor lt 1, \
'' using ($0-0.500):(0.120):3 with labels title ' ' textcolor lt 2,
