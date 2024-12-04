# *********************************************
#    chmod +x train.sh
#    ./train.sh
#    スクリプトに実行権限を与えてから直接実行
#
#   layer毎にジョブを投げる
# ********************************************

#!/bin/bash

for layer in {10..32}; do
    python3 test_net.py $layer
    if [ $? -ne 0 ]; then
        echo "Error occurred during processing of layer $layer. Exiting..."
        exit 1
    fi
done

# integrate all output rootfiles
cd ../../geant/rootfiles
rm -rf output.root
hadd output.root output_*layer.root

# wait until merging is finished, and delete tmp rootfiles
if [ $? -eq 0 ]; then
    echo "Merging finished successfully. Deleting individual files..."
    rm -rf output_*layer.root
else
    echo "Error occurred during merging. Individual files were not deleted. Exiting..."
    exit 1
fi

cd ../../learn/code
python3 latent.py

echo "All process finished!"