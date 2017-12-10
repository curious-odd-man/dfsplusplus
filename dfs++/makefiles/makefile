CC=g++
CXX=g++
RM=rm -f

CPPFLAGS=-Wall -g -std=c++11
LDFLAGS=-Wall -g -std=c++11
LDLIBS=

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
COMMON_SOURCES := $(shell find $(COMMONSRCDIR) -type f -name *.$(SRCEXT))
COMMON_OBJECTS := $(patsubst $(COMMONSRCDIR)/%,$(BUILDDIR)/%,$(COMMON_SOURCES:.$(SRCEXT)=.o))
INC := -I include -I ../common/include


$(TARGET): $(OBJECTS) $(COMMON_OBJECTS)
	@mkdir -p bin
	@echo " Linking..."
	$(CXX) $^ -o $(TARGET) $(LIB)
	
$(BUILDDIR)/%.o: $(COMMONSRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " Compiling common..."
	$(CXX) $(CPPFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " Compiling local..."
	$(CXX) $(CPPFLAGS) $(INC) -c -o $@ $<

.PHONY: 
clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
