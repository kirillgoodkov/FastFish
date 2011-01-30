SRCS   := $(shell find . -name "*.cpp" -print)
VPATH  := $(shell find . -type d -printf "%p:")

CC     = g++
FLAGS  = -iquote ./../FastFish -msse2
CFLAGS = -c -Wall $(FLAGS)
DFLAGS =  -MM -MG $(FLAGS)

PROJNAME = GeoQuery

DBGDIR = ./../_intermediate/$(PROJNAME).GCC.Debug
RELDIR = ./../_intermediate/$(PROJNAME).GCC.Release
DEPDIR = ./../_intermediate/$(PROJNAME).GCC.Depends

DBGDIRL = ./../_intermediate/FastFish.GCC.Debug
RELDIRL = ./../_intermediate/FastFish.GCC.Release

ifeq ($(MAKECMDGOALS),debug)
OUTDIR = $(DBGDIR)
OUTDIRL = $(DBGDIRL)
CFLAGS += -D _DEBUG -g
else
OUTDIR = $(RELDIR)
OUTDIRL = $(RELDIRL)
CFLAGS += -O3
endif

SRCSF  = $(notdir $(SRCS))
OBJS   = $(addprefix $(OUTDIR)/, $(patsubst %.cpp, %.o, $(SRCSF)))
DEPS   = $(addprefix $(DEPDIR)/, $(patsubst %.cpp, %.d, $(SRCSF)))

$(shell mkdir -p $(DBGDIR) $(RELDIR) $(DEPDIR))

all : $(OUTDIR)/$(PROJNAME)

$(OUTDIR)/$(PROJNAME) : $(OBJS) $(OUTDIRL)/libFastFish.a
	$(CC) $(FLAGS) $(OBJS) $(OUTDIRL)/libFastFish.a -o $@ 

$(OUTDIR)/%.o : DEPFILE = $(addprefix $(DEPDIR)/, $(patsubst %.o, %.d, $(@F)))
$(OUTDIR)/%.o : %.cpp
	$(CC) $(DFLAGS) $< -MF"$(DEPFILE)" -MT"$(DEPFILE)"  -MT"$@" 
	$(CC) $(CFLAGS) $< -o $@

debug : $(OUTDIR)/$(PROJNAME)

clean::
	rm -f $(DBGDIR)/*.*
	rm -f $(RELDIR)/*.*
	rm -f $(DEPDIR)/*.*


-include $(DEPS)

