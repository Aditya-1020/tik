CXX = g++

SRCDIR = src
INCDIR = include
OBJDIR = obj

CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I$(INCDIR)

TARGET = TIK

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean