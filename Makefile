CC = g++ 
CFLAGS = -std=c++11 -g -Wall -Wextra -Wunused -fopenmp -Wl,--as-needed

ZBASE=./zbase# 
LIBSO=  -lgomp  -lm 
LIBS=   -lboost_system -lboost_thread -lboost_program_options -lboost_timer  -lboost_chrono -Bdynamic -lgsl -lgslcblas
LIBPATH= $(ZBASE)/lib

DEPS = 

INCLUDES = -I$(ZBASE)

OBJ =  corr cnfbd 

all:	 $(OBJ)  clear mvbin


# compile libraries --------------------------------
semantic.o: semantic.cc $(DEPS)
	$(CC) -c -o $@ $< -fopenmp $(CFLAGS) $(INCLUDES); ar rcs libsemcrct.a semantic.o; rm semantic.o; mv libsemcrct.a ./lib

config.o: config.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS); ar rcs libconfigfile.a config.o; rm config.o; mv libconfigfile.a ./lib

# compile binaries --------------------------------

corr.o: corr.cc
	$(CC) $(INCLUDES) -c -o $@ $< $(CFLAGS)

corr: corr.o
	$(CC)  $(CFLAGS) -o $@ $^  $(LIBSO) $(LIBS) 
#-Bdynamic -lgsl 


cnfbd.o: cnfbd.cc
	$(CC) $(INCLUDES) -c -o $@ $< $(CFLAGS)

cnfbd: cnfbd.o
	$(CC)  $(CFLAGS) -o $@ $^  $(LIBSO) $(LIBS) 
#-Bdynamic -lgsl 


Quantizer.o: Quantizer.cpp
	$(CC) $(INCLUDES) -c -o $@ $< $(CFLAGS)

Quantizer: Quantizer.o
	$(CC)  $(CFLAGS) -o $@ $^ -lsemcrct -lconfigfile -L$(LIBPATH) $(LIBSO) $(LIBS)    


# utility commands --------------------------------

clear:	
	rm -rf *.o
clean:	
	rm -rf *.o *~ $(OBJ) *.tgz

mvbin:
	rm -rf ./bin >&/dev/null; mkdir bin; mv $(OBJ) ./bin
src:
	tar -czvf src.tgz *.cc *.h Makefile Doxyfile
ref:
	doxygen Doxyfile; cd doc/latex; make; make

 # EOF --------------------------------
