# *********************************************
#    chmod +x accuracy.sh
#    ./accuracy.sh
#    スクリプトに実行権限を与えてから直接実行
#
#   layer毎にジョブを投げる
# ********************************************

#!/bin/bash

# # Compare Eff vs FoM for Likelihood and ML at each number of layers
# for layer in {10..32}; do
#     bsub -q sx root -q -b "compare_likelihood.cc($layer)"
# done

# # wait until all jobs are finished
#     bjobs -w | grep -q "PEND\|RUN"
#     while [ $? -eq 0 ]
#     do
#         echo "Waiting for all jobs to finish..."
#         sleep 30
#         bjobs -w | grep -q "PEND\|RUN"
#     done

# calculate Likelihood accuracy for each layer
for layer in {10..32}; do
    bsub -q sx root -q -b "cal_acc.cc($layer)"
done
