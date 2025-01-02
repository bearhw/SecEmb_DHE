
conda activate pyth310  

mkdir RESULTS_OUTPUT


# cmd="gramine-direct ./pytorch" # SGX emulate
cmd="gramine-sgx  ./pytorch" # SGX
cmd="python" # not SGX

core=0

for THR in 16 # threads
do 
    for BATCH in 1 8
    # for BATCH in 16
    do

        CUSTOM_THREAD_COUNT=$THR    taskset -c $core-$(( $core+$THR-1 ))     $cmd inference.py   --ext-type 'index' --batch-size $BATCH 2>&1 | tee RESULTS_OUTPUT/output_inference_index_threads_$THR\_batch_$BATCH.log    
        CUSTOM_THREAD_COUNT=$THR    taskset -c $core-$(( $core+$THR-1 ))     $cmd inference.py   --ext-type 'dhe'   --batch-size $BATCH 2>&1 | tee RESULTS_OUTPUT/output_inference_dhe_threads_$THR\_batch_$BATCH.log    
        CUSTOM_THREAD_COUNT=$THR    taskset -c $core-$(( $core+$THR-1 ))     $cmd inference.py   --ext-type 'ls'    --batch-size $BATCH 2>&1 | tee RESULTS_OUTPUT/output_inference_ls_threads_$THR\_batch_$BATCH.log    
        CUSTOM_THREAD_COUNT=$THR    taskset -c $core-$(( $core+$THR-1 ))     $cmd inference.py   --ext-type 'ztco'  --batch-size $BATCH 2>&1 | tee RESULTS_OUTPUT/output_inference_ztco_threads_$THR\_batch_$BATCH.log    
        CUSTOM_THREAD_COUNT=$THR    taskset -c $core-$(( $core+$THR-1 ))     $cmd inference.py   --ext-type 'ztpo'  --batch-size $BATCH 2>&1 | tee RESULTS_OUTPUT/output_inference_ztpo_threads_$THR\_batch_$BATCH.log    
      
    done 

done
