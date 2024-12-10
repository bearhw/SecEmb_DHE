
.PHONY: all clean

all: Sample_App Test_Correctness

# Util_Cpp_Files := ../App.cpp ../LocalStorage.cpp ../RandomRequestSource.cpp ../Block.cpp ../Bucket.cpp ../Stash.cpp ../ORAMTree.cpp ../PathORAM_Enclave.cpp
# Util_Cpp_Objects := $(Util_Cpp_Files:.cpp=.o)

# Util_Asm_Files := ../oblivious_functions.asm
# Util_Asm_Objects := $(Util_Asm_Files:.asm=.o)


# Sample_App.o : Sample_App.cpp
# 	# g++ -std=c++11 Sample_App.cpp utils.cpp -L=$(CURDIR) -lZT -Wl,--rpath=$(CURDIR) -o sampleapp
# 	g++ -m64 -O0 -g -fPIC -Wno-attributes -std=c++11 -c Sample_App.cpp -o Sample_App.o
# 	@echo "Sample_App.o Done"

# Utils.o : utils.cpp
# 	g++ -m64 -O0 -g -fPIC -Wno-attributes -std=c++11 -c utils.cpp -o utils.o
# 	@echo "utils.o Done"

# sampleapp :  Sample_App.o utils.o $(Util_Asm_Objects) $(Util_Cpp_Objects)
# 	g++ -o sampleapp Sample_App.o utils.o $(Util_Asm_Objects) $(Util_Cpp_Objects)
# 	@echo "Linked"

Sample_App : Sample_App.cpp
	g++ -std=c++11 Sample_App.cpp utils.cpp -L=$(CURDIR) -lZT -O3 -lcrypto -Wl,--rpath=$(CURDIR) -o sampleapp
	

Test_Correctness : Test_Correctness.cpp utils.cpp
	g++ -std=c++11 Test_Correctness.cpp utils.cpp -L=$(CURDIR) -lZT -O3 -Wl,--rpath=$(CURDIR) -o testcorrectness
