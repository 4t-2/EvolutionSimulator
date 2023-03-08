sizex=1920
sizey=1080/5

set term png size sizex, sizey

set output "cpd.png"
plot "cpd.txt" with lines

set output "csigd.png"
plot "csigd.txt" with lines

set output "csped.png"
plot "csped.txt" with lines

set output "csizd.png"
plot "csizd.txt" with lines

