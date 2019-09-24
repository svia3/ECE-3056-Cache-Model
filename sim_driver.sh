BLOCK_SIZES="32 64 128 256 512"
CACHE_SIZES="131072" #128KB
ASSOCIATIVITIES="2 4 8"
TRACES=`ls *.trace`

for b in $BLOCK_SIZES; do
  for s in $CACHE_SIZES; do
    for a in $ASSOCIATIVITIES; do
    echo -n "$b, $s, $a, "
      for t in $TRACES; do
        #echo -n "$t, $b, $s, $a, "
        ./cachesim $t $b $s $a
      done
    printf '\n'
    done
  done
done
