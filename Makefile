CXX = g++

SRCDIR = src
INCDIR = include
OBJDIR = obj

CXXFLAGS = -std=c++17 -Wall -Wextra -I$(INCDIR)

#optimizations
CXXFLAGS += -O3 -march=native -mtune=native -flto=auto -ffast-math
CXXFLAGS += -DNDEBUG -funroll-loops -finline-functions

CXXFLAGS += -pthread
LDFLAGS = -lpthread

TARGET = TIK

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean