# *********************************************
# $ chmod +x mkdata.sh
# $ tmux
# $ ./mkdata.sh
# スクリプトに実行権限を与えてから直接実行
# ********************************************

#!/bin/bash

# # create input data for each layer
# for layer in {10..32}; do
#     bsub -q s root -q -b "mkinput.cc($layer)"
# done

# #wait until all jobs are finished
# bjobs -w | grep -q "PEND\|RUN"
# while [ $? -eq 0 ]
# do
#     echo "Waiting for all jobs to finish..."
#     sleep 60
#     bjobs -w | grep -q "PEND\|RUN"
# done

# integrate all output rootfiles
cd /home/had/kohki/work/ML/2024/geant/rootfiles
rm -rf input.root
hadd input.root input*layer.root

# # wait until merging is finished, and delete tmp rootfiles
# if [ $? -eq 0 ]; then
#     echo "Merging finished successfully. Deleting individual files..."
#     rm -rf output_*layer.root
# else
#     echo "Error occurred during merging. Individual files were not deleted. Exiting..."
#     exit 1
# fi

# # create pdf data for each layer
# for layer in {10..32}; do
#     bsub -q hx root -q -b "mkpdf.cc($layer)"
# done
