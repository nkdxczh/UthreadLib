# specify our compiler and build/link flags...  We use -MMD for dependencies
CXX       = g++
LDFLAGS   = -lrt -lpthread -m32
CXXFLAGS  = -fno-strict-aliasing -Wall -D_REENTRANT -MMD -msse2 -mfpmath=sse \
            -march=core2 -mtune=core2 -m32 -ggdb -O0

# specify paths and filenames...  everything goes into OBJDIR
OBJDIR    = ./obj
BASENAMES = testA uthread
SOURCES   = $(patsubst %, %.cpp, $(BASENAMES)))
OFILES    = $(patsubst %, $(OBJDIR)/%.o, $(BASENAMES) )
DEPS      = $(patsubst %, $(OBJDIR)/%.d, $(BASENAMES) )
GOAL      = $(OBJDIR)/testA

# build rules
.PHONY: all clean realclean info

all: info $(OBJDIR) $(GOAL)
	@echo $(GOAL) complete

# It is useful to print the FLAGS, since we hide the actual CC invocations
info:
	@echo "Building with CXXFLAGS=${CXXFLAGS}"
	@echo "and LDFLAGS=${LDFLAGS}"

# /clean/ deletes everything from the obj directory
clean:
	@rm -f $(OFILES) $(GOAL)

# /realclean/ also kills the directory and the dependencies
realclean:
	@rm -rf $(OBJDIR)

# build the directory in which the .o files will go
$(OBJDIR):
	mkdir -p $@

# compilation rule for making a .o file from a cpp
$(OBJDIR)/%.o: %.cpp
	@echo [${CXX}] $< "-->" $@
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# build the executable
$(GOAL): $(OFILES)
	@echo [${CXX}] "${OBJDIR}/*.o -->" $@
	@$(CXX) -o $@ $^ $(LDFLAGS)

# convince /make/ not to worry if a .d is missing
$(DEPS):

-include $(DEPS)
