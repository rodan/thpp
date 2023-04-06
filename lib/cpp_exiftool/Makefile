CPP = g++
OPT = -g
INC = -I inc

OBJ = src/ExifTool.o src/ExifToolPipe.o src/TagInfo.o
HDR = inc/ExifTool.h inc/ExifToolPipe.h inc/TagInfo.h

all:    example/example1 example/example2 example/example3 example/example4\
        example/example5

clean:
	rm -f example/example? example/*.o src/*.o

example/example1: example/example1.o $(OBJ)
	$(CPP) $(OPT) -o example/example1 example/example1.o $(OBJ)

example/example2: example/example2.o $(OBJ)
	$(CPP) $(OPT) -o example/example2 example/example2.o $(OBJ)

example/example3: example/example3.o $(OBJ)
	$(CPP) $(OPT) -o example/example3 example/example3.o $(OBJ)

example/example4: example/example4.o $(OBJ)
	$(CPP) $(OPT) -o example/example4 example/example4.o $(OBJ)

example/example5: example/example5.o $(OBJ)
	$(CPP) $(OPT) -o example/example5 example/example5.o $(OBJ)

%.o: %.cpp $(HDR)
	$(CPP) $(OPT) $(INC) -o $@ -c $<
