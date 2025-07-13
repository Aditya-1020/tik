CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I$(INCDIR)
TARGET = tik
SRCDIR = src
INCDIR = include
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/receive.cpp $(SRCDIR)/parser.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean