
PROJ = thpp
INCLUDES  := -I ./ -I ../lodepng -I ../tlpi-lib
LOCAL_SRC_CPP := $(wildcard *.cpp)
LOCAL_SRC_C := $(wildcard *.c)
PROJ_SRC   = main.c
SRC        = $(PROJ_SRC) $(LOCAL_SRC_C) $(LOCAL_SRC_CPP)
EXOBJECT   = $(PROJ).o
EXOUTPUT   = $(PROJ)

LODEPNG_DIR = ../lodepng
LODEPNG_LIB = ../lodepng/lodepng.o

TLPI_DIR  = ../tlpi-lib
TLPI_LIB  = ${TLPI_DIR}/libtlpi.a

LOCAL_OBJECTS     = $(LOCAL_SRC_CPP:%.cpp=%.o) $(LOCAL_SRC_C:%.c=%.o)

# Append specific CFLAGS/LDFLAGS
CFLAGS_REL	 += -O3 -fPIC -I. -pedantic
LDFLAGS_REL	 += -Wl,-s
CFLAGS_DBG	 += -O1 -ggdb
LDFLAGS_DBG	 +=

DEFAULT_FLAGS=-DNDEBUG -fopenmp -Wall -Wextra -Wa,-ahl=$(@:.o=.s)
DEFAULT_LDFLAGS=-fopenmp -lm

# First set the flags to their defaults if not supplied externally.
CFLAGS?=$(DEFAULT_FLAGS)
CXXFLAGS?=$(DEFAULT_FLAGS)
LDFLAGS?=$(DEFAULT_LDFLAGS)

CXXFLAGS+=-fPIC -I. -std=c++0x

DEBUG := $(shell grep "^#define CONFIG_DEBUG" config.h)
ifeq ($(DEBUG),)
TARGET	:= RELEASE
CFLAGS	+= $(CFLAGS_REL)
LDFLAGS	+= $(LDFLAGS_REL)
else
TARGET	:= DEBUG
CFLAGS	+= $(CFLAGS_DBG)
LDFLAGS	+= $(LDFLAGS_DBG)
endif

#COVERITY_SCAN_TOKEN := $(shell cat ../.secrets.COVERITY_SCAN_TOKEN)

.PHONY: all
.PHONY: clean
.PHONY: tags
.PHONY: lib

all : lib version.h $(EXOUTPUT) tags

lib : $(LODEPNG_LIB)

$(LODEPNG_LIB): force_look
	@echo "looking into subdir $(LODEPNG_DIR)"
	@$(MAKE) -C $(LODEPNG_DIR) -j1 $(MFLAGS)

$(TLPI_LIB): force_look
	@echo "looking into subdir $(TLPI_DIR)"
	@$(MAKE) -C $(TLPI_DIR) -j1 $(MFLAGS)

version.h: $(SRC)
	@./tools/setver.sh

%.o: %.cpp
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(CFLAGS) -c $< -o $@

%.o: %.c
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(EXOUTPUT): $(LOCAL_OBJECTS) $(LODEPNG_LIB) $(TLPI_LIB) $(EXOBJECT)
	@echo -e "\n>> Building $@ as target $(TARGET)"
	$(CXX) $(LDFLAGS) $(LOCAL_OBJECTS) $(LODEPNG_LIB) $(TLPI_LIB) -o $@ && size $@

$(EXOBJECT): $(PROJ_SRC)
	@$(CXX) $(INCLUDES) $(CFLAGS) -g -c $< -o $@

allgen : ${GEN_EXE}

clean : 
	@${RM} ${EXE} *.o ${PROJ} tags ${TLPI_LIB} *.png *.s
	@${RM} -rf cov-int

tags: $(SRCS)
	@echo "Generating tags .."
	@exuberant-ctags -R

${EXE} : ${TLPI_LIB}		# True as a rough approximation

cppcheck: $(PROJ)
	@cppcheck --quiet --force --enable=warning --enable=performance --enable=information --enable=performance --enable=portability -I ./ ./

scan-build: clean
	@scan-build make

coverity: clean
	@cov-build --dir cov-int make
	@tar czvf /tmp/$(PROJ).tgz cov-int
	@curl --form token=$(COVERITY_SCAN_TOKEN) \
	  --form email=2b4eda@subdimension.ro \
	  --form file=@/tmp/$(PROJ).tgz \
	  --form version="$(shell ./tools/getver.sh -i ./version.h -t MAJ.MINbBUILD)" \
	  --form description="Description" \
	  https://scan.coverity.com/builds?project=rodan%2Fsigdup

force_look:
	@true
