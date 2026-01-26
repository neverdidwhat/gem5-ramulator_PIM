for i in $(seq 1 16); do
  numactl --cpunodebind=0 --membind=0 ./s1mple > /tmp/bw_$i.txt &
done
wait
grep Bandwidth /tmp/bw_*.txt
