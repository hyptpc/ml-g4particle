# *********************************************
#    chmod +x train.sh
#    ./train.sh
#    スクリプトに実行権限を与えてから直接実行

#   * run reg_tuning.py before this script, and check if reg_parameters.json exists
#
#
# ********************************************

#!/bin/bash

for layer in {10..32}; do
    bsub -q h python3 train_reg.py $layer
done

# for layer in {10..32}; do
#     bsub -q h python3 test_reg.py $layer
# done
