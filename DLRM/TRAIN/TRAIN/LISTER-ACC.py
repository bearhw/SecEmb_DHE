

import glob



import os

# files = glob.glob("train*e64*.log")
# files = glob.glob("train*99999.log")
files = glob.glob("train*tera*.log")
files = glob.glob("train*select*.log")
files = glob.glob("train*.log")

# files.sort(key=os.path.getmtime)
# print("\n".join(files))

files = sorted(files)


for name in files: 
     
    f = open(name, "r")
    contents = f.readlines()

    for c in contents:      # last valid line automatically
        if '%, best ' in c:
            acc=c.split(' ')[5].strip()

    print(name,'          ',acc)


