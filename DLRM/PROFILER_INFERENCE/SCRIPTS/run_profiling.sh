
conda activate pyth310


cmd="gramine-sgx  ./pytorch"
cmd="python"



core=0

####################
####################

for t in 1 2 3 4 8 # threads
do
    echo "Threads $t "

    ## non-secure lookup 

    for i in 5 10 50 100 500 750 1000 2000 3000 4000 5000 6000 7000 10000 12500 15000 20000 50000 75000 100000 125000 150000 # tableSize
    do
      echo "Table Size $i "

          CUSTOM_THREAD_COUNT=$t   TABLE_SIZE_ROWS=$i   taskset -c $core-$(( $core+$t-1 ))    $cmd   run_dlrm_e64.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/output_dlrm_e64__threads_$t\_tableSize_$i.log
          CUSTOM_THREAD_COUNT=$t   TABLE_SIZE_ROWS=$i   taskset -c $core-$(( $core+$t-1 ))    $cmd   run_dlrm_e16.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/output_dlrm_e16__threads_$t\_tableSize_$i.log

    done

done

 



####################
####################

for t in 1 2 3 4 8 # threads
do
    echo "Threads $t "

    ## DHE uniform

    CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))  $cmd run_dhe_e64.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/output_dhe_e64__threads_$t.log        
    CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))  $cmd run_dhe_e16.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/output_dhe_e16__threads_$t.log        

    ## DHE varying

    CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))  $cmd run_dhe_e64_varying.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/output_dhe_e64_varying__threads_$t.log        
    CUSTOM_THREAD_COUNT=$t  taskset -c $core-$(( $core+$t-1 ))  $cmd run_dhe_e16_varying.py --inference --device 'cpu'        | tee RESULTS_OUTPUT/output_dhe_e16_varying__threads_$t.log        

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

          CUSTOM_THREAD_COUNT=$t   TABLE_SIZE_ROWS=$i   taskset -c $core-$(( $core+$t-1 ))        $cmd   run_dlrmExt_e64.py --inference --device 'cpu'   --ext-type 'linearscan'     | tee RESULTS_OUTPUT/output_dlrmExt_e64_ls__threads_$t\_tableSize_$i.log
          CUSTOM_THREAD_COUNT=$t   TABLE_SIZE_ROWS=$i   taskset -c $core-$(( $core+$t-1 ))        $cmd   run_dlrmExt_e16.py --inference --device 'cpu'   --ext-type 'linearscan'     | tee RESULTS_OUTPUT/output_dlrmExt_e16_ls__threads_$t\_tableSize_$i.log

    done

done
 



####################
####################

for t in 1 # threads
do
    echo "Threads $t "

    ## ORAM

    for i in 5 10 50 100 500 750 1000 2000 3000 4000 5000 6000 7000 10000 12500 15000 20000 50000 75000 100000 125000 150000 # tableSize
    do
      echo "Table Size $i "

        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$t-1 ))       $cmd   run_dlrmExt_e64.py --inference --device 'cpu'   --ext-type 'zt_co'     | tee RESULTS_OUTPUT/output_dlrmExt_e64_ztco__tableSize_$i.log
        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$t-1 ))       $cmd   run_dlrmExt_e64.py --inference --device 'cpu'   --ext-type 'zt_po'     | tee RESULTS_OUTPUT/output_dlrmExt_e64_ztpo__tableSize_$i.log  

        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$t-1 ))       $cmd   run_dlrmExt_e16.py --inference --device 'cpu'   --ext-type 'zt_co'     | tee RESULTS_OUTPUT/output_dlrmExt_e16_ztco__tableSize_$i.log
        CUSTOM_THREAD_COUNT=1    TABLE_SIZE_ROWS=$i  taskset -c $core-$(( $core+$t-1 ))       $cmd   run_dlrmExt_e16.py --inference --device 'cpu'   --ext-type 'zt_po'     | tee RESULTS_OUTPUT/output_dlrmExt_e16_ztpo__tableSize_$i.log  

    done

done

