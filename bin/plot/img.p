sizex=1920
sizey=1080/2

set term png size sizex, sizey

set output "plot.png"
plot "neat.txt" with lines, "rl.txt" with lines, "both.txt" with lines
