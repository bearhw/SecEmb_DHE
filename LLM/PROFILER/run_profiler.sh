
conda activate pyth310  

mkdir RESULTS_OUTPUT


# cmd="gramine-direct ./pytorch" # SGX emulate
cmd="gramine-sgx  ./pytorch" # SGX
cmd="python" # not SGX

core=0

for TABLE_SIZE in 50257x768 50257x1024 50257x1536 50257x2048 50257x3072 50257x4096 50257x5120 50257x8192
# for TABLE_SIZE in 50257x1024  
do
    # threads    
    THR=16  
    
    CUSTOM_THREAD_COUNT=$THR   taskset -c $core-$(( $core+$THR-1 ))     $cmd profiler.py --ext-type 'index'       --embTableSize $TABLE_SIZE     2>&1   | tee RESULTS_OUTPUT/output_index_$TABLE_SIZE\_threads_$THR.log     
    CUSTOM_THREAD_COUNT=$THR   taskset -c $core-$(( $core+$THR-1 ))     $cmd profiler.py --ext-type 'dhe'         --embTableSize $TABLE_SIZE     2>&1   | tee RESULTS_OUTPUT/output_dhe_$TABLE_SIZE\_threads_$THR.log         
    CUSTOM_THREAD_COUNT=$THR   taskset -c $core-$(( $core+$THR-1 ))     $cmd profiler.py --ext-type 'ls'          --embTableSize $TABLE_SIZE     2>&1   | tee RESULTS_OUTPUT/output_ls_$TABLE_SIZE\_threads_$THR.log            
    CUSTOM_THREAD_COUNT=$THR   taskset -c $core-$(( $core+$THR-1 ))     $cmd profiler.py --ext-type 'ztco'        --embTableSize $TABLE_SIZE     2>&1   | tee RESULTS_OUTPUT/output_ztco_$TABLE_SIZE\_threads_1.log      # 
    CUSTOM_THREAD_COUNT=$THR   taskset -c $core-$(( $core+$THR-1 ))     $cmd profiler.py --ext-type 'ztpo'        --embTableSize $TABLE_SIZE     2>&1   | tee RESULTS_OUTPUT/output_ztpo_$TABLE_SIZE\_threads_1.log      #   

done


