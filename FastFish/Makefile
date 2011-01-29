SRCS   := $(shell find . -name "*.cpp" -print)
VPATH  := $(shell find . -type d -printf "%p:")

CC     = g++
FLAGS  = -iquote . -msse2
CFLAGS = -c -Wall $(FLAGS)
DFLAGS =  -MM -MG $(FLAGS)


DBGDIR = ./../_intermediate/FastFish.GCC.Debug
RELDIR = ./../_intermediate/FastFish.GCC.Release
DEPDIR = ./../_intermediate/FastFish.GCC.Depends

ifeq ($(MAKECMDGOALS),debug)
OUTDIR = $(DBGDIR)
CFLAGS += -D _DEBUG -g
else
OUTDIR = $(RELDIR)
CFLAGS += -O3
endif

SRCSF  = $(notdir $(SRCS))
OBJS   = $(addprefix $(OUTDIR)/, $(patsubst %.cpp, %.o, $(SRCSF)))
DEPS   = $(addprefix $(DEPDIR)/, $(patsubst %.cpp, %.d, $(SRCSF)))


$(shell mkdir -p $(DBGDIR) $(RELDIR) $(DEPDIR))

all : $(OUTDIR)/libFastFish.a

$(OUTDIR)/libFastFish.a : $(OBJS)
	ar rcs $@ $?

$(OUTDIR)/%.o : DEPFILE = $(addprefix $(DEPDIR)/, $(patsubst %.o, %.d, $(@F)))
$(OUTDIR)/%.o : %.cpp
	$(CC) $(DFLAGS) $< -MF"$(DEPFILE)" -MT"$(DEPFILE)"  -MT"$@" 
	$(CC) $(CFLAGS) $< -o $@

debug : $(OUTDIR)/libFastFish.a

clean::
	rm -f $(DBGDIR)/*.*
	rm -f $(RELDIR)/*.*
	rm -f $(DEPDIR)/*.*


-include $(DEPS)

