# *********************************************
#    chmod +x train.sh
#    ./train.sh
#    スクリプトに実行権限を与えてから直接実行
#
#   layer毎にジョブを投げる
# ********************************************

#!/bin/bash

for layer in {10..32}; do
    bsub -q hx python3 test_net.py $layer
done

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

# integrate all output rootfiles
cd /home/had/kohki/work/ML/2024/geant/rootfiles
hadd output.root output_*layer.root

# wait until merging is finished, and delete tmp rootfiles
if [ $? -eq 0 ]; then
    echo "Merging finished successfully. Deleting individual files..."
    rm -rf output_*layer.root
else
    echo "Error occurred during merging. Individual files were not deleted. Exiting..."
    exit 1
fi

echo "All process finished!"