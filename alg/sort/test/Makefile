CC = /usr/bin/g++

OBJ1 = test 
LIBDIR = -lpthread -lm
INCLUDEDIR= -I./ -I./include -I../include
VPATH = ../src:../
SORT = mergesort.o insertion_sort.o bubble_sort.o selection_sort.o shell_sort.o quick_sort.o heapsort.o

CPPFLAGS = -g -finline-functions -Wall -Winline -pipe -ffast-math -ftracer  -march=x86-64  -frename-registers -mtune=core2 -fprefetch-loop-arrays -mmmx -fpermissive


all : $(OBJ1) output
	rm  -f *.o
	cp $(OBJ1) ./output

output :
	rm -rf ./outuput
	mkdir ./output
	cp $(OBJ1) ./output

$(OBJ1)	: load.o fun.o $(SORT) time.o
	$(CC) -O2 -o $@ $^  $(INCLUDEDIR)  $(LIBDIR)	

%.o	: %.cpp %.h
	$(CC) -c $< -o $@ $(INCLUDEDIR) $(CPPFLAGS)
    
clean:
	rm -r output $(OBJ1) 
	rm -rf *.o


