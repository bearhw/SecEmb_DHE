  
import glob
import os

logdir = '.' + '/'

# csv header
header_table    = 'Threads,BatchSize,DenseFeatures,TableCount,EmbSize,NumLookups,NULL,TimeTotal,TimeBottom,TimeEmb,TimeInteract,TimeTop'  
header_dhe      = 'Threads,BatchSize,DenseFeatures,TableCount,EmbSize,NumLookups,DheK,TimeTotal,TimeBottom,TimeEmb,TimeInteract,TimeTop,TimeEmbHash,TimeEmbMLP'
  
  
  
  
# SELECT ONE

# THIS
table=1
dhe=0
files = glob.glob(logdir+'*dlrm*_*.log')


# OR THIS
dhe=1
table=0
files = glob.glob(logdir+'*dhe_*.log')
# files = glob.glob(logdir+'*hybrid*.log')





if table == 1:
    header = header_table
else:
    header = header_dhe




word = 'FINAL'      # used to determine relevant part in log, to copy to csv

for file in files:
    print(file)
    input = file

    with open(input, 'r') as fp:
        # read all lines in a list
        lines = fp.readlines()
        found = False
        found_idx = -1
        found_idx_end = -1
        for line in lines:
            if line.find(word) != -1:
                print('Line Number:', lines.index(line))
                found = True 
                found_idx = lines.index(line)

        for i, line in enumerate(lines):
            if found:
                if i > found_idx:
                    if (len(line.strip())) == 0:
                        print(i)
                        found_idx_end = i
                        break
                    
        selected = lines[found_idx+1:found_idx_end]
        selected.insert(0, header+'\n')

        print(selected)

        input = input.replace(logdir,'')
        output = input.replace('.log','.csv')


        f = open(logdir+"ParsedCSVs/"+output, "w")
        f.writelines(selected)
        f.close()

