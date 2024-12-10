#
# Copyright (C) 2011-2017 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Intel Corporation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

NASM ?= nasm
NASM_Flags ?= -f elf64

######## SGX SDK Settings ########

MODE ?= HW
SGX_ARCH ?= x64
# DEBUG ?= 1
DEBUG ?= 0

SGX_COMMON_CFLAGS := -m64

ifeq ($(DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O3
endif

ifeq ($(DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O3 -D_FORTIFY_SOURCE=2
endif


######## App Settings ######## 


# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Cpp_Flags := $(App_C_Flags) -std=c++11



App_Name := app

#################################################
ZT_LIBRARY_PATH := ./Sample_App/

Util_Include_Path := -IInclude -IApp -I/usr/bin/include
Util_Cpp_Files := ZT_Utils/App.cpp ZT_Utils/LocalStorage.cpp ZT_Utils/RandomRequestSource.cpp ZT_Utils/Block.cpp ZT_Utils/Bucket.cpp ZT_Utils/Stash.cpp ZT_Utils/ORAMTree.cpp ZT_Utils/PathORAM_Enclave.cpp ZT_Utils/CircuitORAM_Enclave.cpp
Util_Cpp_Objects := $(Util_Cpp_Files:.cpp=.o)

Util_Asm_Files := ZT_Utils/oblivious_functions.asm
Util_Asm_Objects := $(Util_Asm_Files:.asm=.o)

# COMMON_CFLAGS := -m64 -O0 -g -fPIC -Wno-attributes $(Util_Include_Path)
# Compile_C_Flags := -m64 -O0 -g
COMMON_CFLAGS := -m64 -O3  -fPIC -Wno-attributes $(Util_Include_Path)
Compile_C_Flags := -m64 -O3
COMMON_CPPFLAGS := $(COMMON_CFLAGS) -std=c++11 

Util_Link_Flags := $(COMMON_CFLAGS) -L/usr/bin/include/lib64 -lpthread -lcrypto

##################################################


Build_Mode = HW_DEBUG


.PHONY: all run


all: $(App_Name)
ifeq ($(Build_Mode), HW_DEBUG)
	@echo "The project has been built in debug hardware mode."
endif

run: all

######## App Objects ########

ZT_Utils/oblivious_functions.o: ZT_Utils/oblivious_functions.asm
	@$(NASM) $(NASM_Flags) $< -o $@ 

ZT_Utils/%.o: ZT_Utils/%.cpp $(Util_Asm_Objects)
	@$(CXX) $(COMMON_CPPFLAGS) -c $< -o $@
	@echo "CXX  <=  $<"
	@echo "Done compiling"


$(App_Name): $(Util_Cpp_Objects) $(Util_Asm_Objects)
	#To build a dynamic-linked library ZeroTrace:
	@$(CXX) $^ -shared -o libZT.so $(Util_Link_Flags)
	@echo "LINK =>  $@"
	cp libZT.so Sample_App/ 
	$(MAKE) -C Sample_App/
	@echo "Done"
	

.PHONY: clean

clean:
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(App_Cpp_Objects) $(Util_Cpp_Objects) Sample_App/libZT.so Sample_App/lsclient Sample_App/sampleapp Sample_App/testcorrectness libZT.so

