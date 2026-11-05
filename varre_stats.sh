#!/bin/bash
# Uso: ./varre_stats.sh <raio> <arquivo_saida>
RAIO=$1
SAIDA=$2
REPS=10
echo "nDevices,pdr_medio,desvio" > "$SAIDA"
for N in 10 25 50 100 200 300 500; do
  echo ">>> N=$N (raio=$RAIO)"
  VALORES=""
  for SEED in $(seq 1 $REPS); do
    LINHA=$(./ns3 run "scratch/boa-vista --nDevices=$N --radius=$RAIO --RngRun=$SEED" 2>/dev/null | tail -n1)
    PDR=$(echo "$LINHA" | cut -d',' -f4)
    VALORES="$VALORES $PDR"
  done
  python3 -c "
import statistics as st
v = [float(x) for x in '$VALORES'.split()]
print(f'$N,{st.mean(v):.4f},{st.pstdev(v):.4f}')
" >> "$SAIDA"
done
