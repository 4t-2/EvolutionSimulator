sizex=1920
sizey=1080/2

set term png size sizex, sizey
set output "cpd.png"
plot "cpd.txt" with lines

