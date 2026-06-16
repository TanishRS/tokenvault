CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:.cpp=.o)
TARGET   := tokenvault

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run clean
