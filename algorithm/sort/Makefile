CC = /usr/bin/g++

OBJ1 = sort 
#OBJ2 = list
LIBDIR = -lpthread -lm
INCLUDEDIR= -I./ -I./include -I../include

#CPPFLAGS = -g -finline-functions -Wall -Winline -pipe  -m64 -ffast-math -ftracer  -march=x86-64  -frename-registers -mtune=core2 -mfpmath=sse -fprefetch-loop-arrays -mmmx
CPPFLAGS = -g -finline-functions -Wall -Winline -pipe -ffast-math -ftracer  -march=x86-64  -frename-registers -mtune=core2 -fprefetch-loop-arrays -mmmx -fpermissive


all : $(OBJ1) $(OBJ2) output
	rm  -f *.o
	cp $(OBJ1) $(OBJ2)   ./output

output :
	rm -rf ./outuput
	mkdir ./output
	cp $(OBJ1) $(OBJ2) ./output
$(OBJ1)	: main.o load.o mergesort.o fun.o insertion_sort.o bubble_sort.o
	$(CC) -O2 -o $@ $^  $(INCLUDEDIR)  $(LIBDIR)	
#$(OBJ2)	: list.o load.o sort.o  
#	$(CC) -O2 -o $@ $^  $(INCLUDEDIR)  $(LIBDIR)	
%.o	: %.cpp %.h
	$(CC) -c $< -o $@ $(INCLUDEDIR) $(CPPFLAGS)
    
clean:
	rm -r output $(OBJ1) $(OBJ2) 

