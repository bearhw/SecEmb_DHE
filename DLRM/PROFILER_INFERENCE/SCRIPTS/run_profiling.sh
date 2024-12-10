
conda activate pyth310




####################
####################

for i in 1 2 3 4 8 # threads
do
    echo "Threads $i " 
     
    ## non-secure lookup 

    CUSTOM_THREAD_COUNT=$i   taskset -c $core-$(( $core+$i-1 ))   gramine-sgx  ./pytorch run_dlrm_kaggle_e2e.py --inference --device 'cpu'      2>&1 | tee RESULTS_OUTPUT/sgx_out_dlrm_kaggle__threads_$i.log    
    CUSTOM_THREAD_COUNT=$i   taskset -c $core-$(( $core+$i-1 ))   gramine-sgx  ./pytorch run_dlrm_tera_e2e.py   --inference --device 'cpu'      2>&1 | tee RESULTS_OUTPUT/sgx_out_dlrm_tera__threads_$i.log      

done
 




####################
####################

for i in 1 2 3 4 8 # threads
do
    echo "Threads $i "

    # uniform

    CUSTOM_THREAD_COUNT=$i  taskset -c $core-$(( $core+$i-1 ))  gramine-sgx  ./pytorch run_dhe_e64.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/sgx_out_dhe_e64__threads_$i.log        
    CUSTOM_THREAD_COUNT=$i  taskset -c $core-$(( $core+$i-1 ))  gramine-sgx  ./pytorch run_dhe_e16.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/sgx_out_dhe_e16__threads_$i.log        

    # varying 

    CUSTOM_THREAD_COUNT=$i  taskset -c $core-$(( $core+$i-1 ))  gramine-sgx  ./pytorch run_dhe_e64_varying.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/sgx_out_dhe_e64_varying__threads_$i.log        
    CUSTOM_THREAD_COUNT=$i  taskset -c $core-$(( $core+$i-1 ))  gramine-sgx  ./pytorch run_dhe_e16_varying.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/sgx_out_dhe_e16_varying__threads_$i.log        

done 

 



####################
####################

for t in 1 2 3 4 8 # threads
do
    echo "Threads $t "

    ## linear scan

    for i in 5 10 50 100 500 750 1000 2000 3000 4000 5000 6000 7000 10000 12500 15000 20000 50000 75000 100000 125000 150000 # tableSize
    do
      echo "Table Size $i "

          CUSTOM_THREAD_COUNT=$t   TABLE_SIZE_ROWS=$i taskset -c $core-$(( $core+$i-1 ))        gramine-sgx  ./pytorch   run_dlrmExt_e64.py --inference --device 'cpu'   --ext-type 'linearscan'     | tee RESULTS_OUTPUT/sgx_out_dlrmExt_e64_ls__threads_$t\_tableSize_$i.log
          CUSTOM_THREAD_COUNT=$t   TABLE_SIZE_ROWS=$i taskset -c $core-$(( $core+$i-1 ))        gramine-sgx  ./pytorch   run_dlrmExt_e16.py --inference --device 'cpu'   --ext-type 'linearscan'     | tee RESULTS_OUTPUT/sgx_out_dlrmExt_e16_ls__threads_$t\_tableSize_$i.log

    done

done
 



####################
####################

# thread vary
for t in 1 
do
    echo "Threads $t "

    for i in 5 10 50 100 500 750 1000 2000 3000 4000 5000 6000 7000 10000 12500 15000 20000 50000 75000 100000 125000 150000 # tableSize
    do
      echo "Table Size $i "

        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$i-1 ))       gramine-sgx  ./pytorch   run_dlrmExt_e64.py --inference --device 'cpu'   --ext-type 'zt_co'     | tee RESULTS_OUTPUT/sgx_out_dlrmExt_e64_ztco__tableSize_$i.log
        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$i-1 ))       gramine-sgx  ./pytorch   run_dlrmExt_e64.py --inference --device 'cpu'   --ext-type 'zt_po'     | tee RESULTS_OUTPUT/sgx_out_dlrmExt_e64_ztpo__tableSize_$i.log  

        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$i-1 ))       gramine-sgx  ./pytorch   run_dlrmExt_e16.py --inference --device 'cpu'   --ext-type 'zt_co'     | tee RESULTS_OUTPUT/sgx_out_dlrmExt_e16_ztco__tableSize_$i.log
        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$i-1 ))       gramine-sgx  ./pytorch   run_dlrmExt_e16.py --inference --device 'cpu'   --ext-type 'zt_po'     | tee RESULTS_OUTPUT/sgx_out_dlrmExt_e16_ztpo__tableSize_$i.log  

    done

done

