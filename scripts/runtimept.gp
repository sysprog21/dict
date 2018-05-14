reset
set xlabel 'prefix'
set ylabel 'time(sec)'
set title 'perfomance comparison'
set term png enhanced font 'Verdana,10'
set output 'runtime2.png'
set format x "%10.0f"
set xtic 1200
set xtics rotate by 45 right

plot [:12500][:1000]'bench_cpy.txt' using 1:2 with points title 'cpy',\
'bench_ref.txt' using 1:2 with points title 'ref',\



