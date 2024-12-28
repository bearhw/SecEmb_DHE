  
import glob
import os

logdir = '.' + '/'
if not os.path.exists('ParsedCSVs'):
   os.makedirs('ParsedCSVs')
   
# csv header
header_table    = 'Threads,BatchSize,DenseFeatures,TableCount,EmbSize,NumLookups,NULL,TimeTotal,TimeBottom,TimeEmb,TimeInteract,TimeTop'  
header_dhe      = 'Threads,BatchSize,DenseFeatures,TableCount,EmbSize,NumLookups,DheK,TimeTotal,TimeBottom,TimeEmb,TimeInteract,TimeTop,TimeEmbHash,TimeEmbMLP'
  
  


def extract_res(header,files):

    word = 'FINAL'      # word used to determine relevant part in log, to copy to csv

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




  

files = glob.glob(logdir+'*dlrm*_*.log')
header = header_table
extract_res(header, files)


files = glob.glob(logdir+'*dhe_*.log')
header = header_dhe
extract_res(header, files)

files = glob.glob(logdir+'*hybrid*.log')
header = header_dhe
extract_res(header, files)

 