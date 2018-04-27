#!/bin/bash
port=9999
while true
do
  # count the number of occurrences of port $port in output: 1= in use; 0 = not in use
  result=$(ss -ln src :$port | grep -Ec -e "\<$port\>")
  if [ "$result" -gt 0 ]; then
    ((port++))
  else
    echo "$port"
    exit
  fi
done
