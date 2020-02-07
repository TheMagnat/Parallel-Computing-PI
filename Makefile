CC = g++
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
CFLAGS = -Wall -std=c++17 -O3
LFLAGS = -lgmpxx -lgmp

exec.out:    $(OBJ) $(INCLUDES) 
	$(CC) -o $@ $(OBJ) $(LFLAGS)
	
%.o:	%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)
	
clean:
	rm *.o

cleanall:
	rm *.o exec.out