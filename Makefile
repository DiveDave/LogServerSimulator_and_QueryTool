COMPILER = g++

CFLAG = -g 

#If need traversal all subdirectories, enable this one and disable the above FLAG
#CFLAG=-g -DGOES_TO_SUB_DIRECTORIES 

LFLAG = -pthread

TARGET_GEN=logGenerator
TARGET_QRY=logQuerier

all: logGenerator.o logQuerier.o logFile.o comm.a
	$(COMPILER) $(LFLAG) -o $(TARGET_GEN) logGenerator.o logFile.o comm.a
	$(COMPILER) $(LFLAG) -o $(TARGET_QRY) logQuerier.o logFile.o comm.a

logGenerator.o: logGenerator.cpp logGenerator.h
	$(COMPILER) $(CFLAG) -c logGenerator.cpp

logQuerier.o: logQuerier.cpp
	$(COMPILER) $(CFLAG) -c logQuerier.cpp

logFile.o: logFile.cpp
	$(COMPILER) $(CFLAG) -c logFile.cpp

#logFile.a: logFile.o
#	ar -rcs logFile.a logFile.o

comm.a: comm.o
	ar -rcs comm.a comm.o


clean:
	rm -rf *.o *.a $(TARGET_GEN) $(TARGET_QRY)

