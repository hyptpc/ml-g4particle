# *********************************************
#    chmod +x train.sh
#    ./train.sh
#    スクリプトに実行権限を与えてから直接実行
#
#    conda activate py37
#
# ********************************************

#!/bin/bash

# ================================
# 1. Regression training for dE/dx -> β
# ================================

python3 tuning_reg.py

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

# レイヤーごとの訓練ジョブを送信
for layer in {10..32}; do
    bsub -q h python3 train_reg.py $layer
done

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

# ================================
# 2. Check regression accuracy
# ================================

for layer in {10..32}; do
    bsub -q h python3 test_reg.py $layer
done

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

# ================================
# 3. Classification training for PID
# ================================

python3 tuning_class.py

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

for layer in {10..32}; do
    bsub -q h python3 train_class.py $layer
done

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

# ================================
# 4. Check classification accuracy
# ================================

for layer in {10..32}; do
    python3 test_class.py $layer
    if [ $? -ne 0 ]; then
        echo "Error occurred during processing of layer $layer. Exiting..."
        exit 1
    fi
done

bjobs -w | grep -q "PEND\|RUN"
while [ $? -eq 0 ]
do
    echo "Waiting for all jobs to finish..."
    sleep 60
    bjobs -w | grep -q "PEND\|RUN"
done

cd ../../geant/data
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
