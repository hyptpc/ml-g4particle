# *********************************************
#    chmod +x train.sh
#    ./train.sh
#    スクリプトに実行権限を与えてから直接実行
#
#   layer毎にジョブを投げる
# ********************************************

#!/bin/bash

for layer in {10..32}; do
    bsub -q hx python3 train_net.py $layer
done
