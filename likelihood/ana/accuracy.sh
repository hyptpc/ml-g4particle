# *********************************************
#    chmod +x accuracy.sh
#    ./accuracy.sh
#    スクリプトに実行権限を与えてから直接実行
#
#   layer毎にジョブを投げる
# ********************************************

#!/bin/bash

# Submit jobs to compare Eff vs FoM for Likelihood and ML at each number of layers
for layer in {10..32}; do
    bsub -q sx root -q -b "compare_likelihood.cc($layer)"
done

# Wait for all the jobs to finish
while true; do
    # Check if there are any running or pending jobs submitted by this user
    running_jobs=$(bjobs | grep -c "PEND\|RUN")
    
    if [ "$running_jobs" -eq 0 ]; then
        break
    else
        echo "Waiting for jobs to finish..."
        sleep 60  # Check every 60 seconds
    fi
done

# After all jobs are finished, submit the next set of jobs to calculate Likelihood accuracy
for layer in {10..32}; do
    bsub -q sx root -q -b "cal_acc.cc($layer)"
done

echo "All jobs submitted."
