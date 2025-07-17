CXX = g++

SRCDIR = src
INCDIR = include

CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I$(INCDIR)

TARGET = tik

SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/receive.cpp $(SRCDIR)/parser.cpp $(SRCDIR)/orders.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean