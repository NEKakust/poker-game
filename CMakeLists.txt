CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC = src/Deck.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
all: poker
poker: $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o poker
clean:
	rm -f poker *.o src/*.o
