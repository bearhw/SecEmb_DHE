
conda activate pyth310

mkdir RESULTS_OUTPUT


cmd="gramine-sgx  ./pytorch" # SGX
cmd="python" # not SGX



core=0



####################
# end-end
####################



# threads
for t in 1 # 2 3 4 8
do
    echo "Threads $t "    
    
    ## non-secure lookup 

    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrm_tera_e2e.py   --inference --device 'cpu'      2>&1 | tee RESULTS_OUTPUT/output_dlrm_tera__threads_$t.log     
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrm_kaggle_e2e.py --inference --device 'cpu'      2>&1 | tee RESULTS_OUTPUT/output_dlrm_kaggle__threads_$t.log     
     

    ## DHE uniform
    
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dhe_tera_e2e.py   --inference --device 'cpu'     2>&1   | tee RESULTS_OUTPUT/output_dhe_tera__threads_$t.log     
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dhe_kaggle_e2e.py --inference --device 'cpu'     2>&1  | tee RESULTS_OUTPUT/output_dhe_kaggle__threads_$t.log     

    ## DHE varying
    
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dhe_tera_e2e.py   --inference --device 'cpu'   --dhe-varying    2>&1 | tee RESULTS_OUTPUT/output_dhe_varying_tera__threads_$t.log     
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dhe_kaggle_e2e.py --inference --device 'cpu'   --dhe-varying    2>&1 | tee RESULTS_OUTPUT/output_dhe_varying_kaggle__threads_$t.log     
    



done
 

## hybrid

# threads
for t in 1 # 2 3 4 8 
do
    echo "Threads $t "

    for threshold in 2000 3000 5000 5670 10000     14991 93144 120000      150000   # kaggle e16                              
    do

        CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))   $cmd  run_hybrid_kaggle.py --inference --device 'cpu'   --table-size-threshold=$threshold                   2>&1  | tee RESULTS_OUTPUT/output_kaggle_hybrid_threshold$threshold\_threads_$t.log      
        CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))   $cmd  run_hybrid_kaggle.py --inference --device 'cpu'   --table-size-threshold=$threshold  --dhe-varying    2>&1  | tee RESULTS_OUTPUT/output_kaggle_hybrid_varying_threshold$threshold\_threads_$t.log      
    
    done
    
    for threshold in 120 500 1000 2000   5000   7111 7377 11155 12419 17216 20133 28000           # tera e64      
    do

        CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))  $cmd  run_hybrid_tera.py --inference --device 'cpu'   --table-size-threshold=$threshold                     2>&1  | tee RESULTS_OUTPUT/output_tera_hybrid_threshold$threshold\_threads_$t.log      
        CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))  $cmd  run_hybrid_tera.py --inference --device 'cpu'   --table-size-threshold=$threshold  --dhe-varying      2>&1  | tee RESULTS_OUTPUT/output_tera_hybrid_varying_threshold$threshold\_threads_$t.log      
    
    done
done




## extensions based (LS/ORAM)

# threads
for t in 1 # 2 3 4 8
do
    echo "Threads $t "    
    
    ## linear scan

    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrmExt_tera_e2e.py   --inference --device 'cpu'  --ext-type 'linearscan'    2>&1 | tee RESULTS_OUTPUT/output_dlrmExt_ls_tera__threads_$t.log     
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrmExt_kaggle_e2e.py --inference --device 'cpu'  --ext-type 'linearscan'    2>&1 | tee RESULTS_OUTPUT/output_dlrmExt_ls_kaggle__threads_$t.log     
         
    ## ORAM  

    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrmExt_tera_e2e.py   --inference --device 'cpu'  --ext-type 'zt_po'    2>&1 | tee RESULTS_OUTPUT/output_dlrmExt_ztpo_tera__threads_$t.log     
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrmExt_kaggle_e2e.py --inference --device 'cpu'  --ext-type 'zt_po'    2>&1 | tee RESULTS_OUTPUT/output_dlrmExt_ztpo_kaggle__threads_$t.log     

    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrmExt_tera_e2e.py   --inference --device 'cpu'  --ext-type 'zt_co'    2>&1 | tee RESULTS_OUTPUT/output_dlrmExt_ztco_tera__threads_$t.log     
    CUSTOM_THREAD_COUNT=$t   taskset -c $core-$(( $core+$t-1 ))   $cmd run_dlrmExt_kaggle_e2e.py --inference --device 'cpu'  --ext-type 'zt_co'    2>&1 | tee RESULTS_OUTPUT/output_dlrmExt_ztco_kaggle__threads_$t.log     
          
done
 




