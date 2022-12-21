#!/usr/bin/env bash

IN="$(search .cpp)"

mails=$(echo $IN | tr ";" "\n")

for addr in $mails
do
	let total="$(cat $addr | wc -l)"+total
done

echo $total
