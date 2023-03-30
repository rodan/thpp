
PROJ = thpp

LODEPNG_DIR = ../lib/lodepng
LODEPNG_LIB = ${LODEPNG_DIR}/lodepng.o
LODEPNG_SRC_C = ${LODEPNG_DIR}/lodepng.cpp

TINYTIFF_DIR = ../lib/TinyTIFF/src
TINYTIFF_LIB = ${TINYTIFF_DIR}/tinytiffreader.o
TINYTIFF_SRC_C = ${TINYTIFF_DIR}/tinytiffreader.c

TLPI_DIR  = ../lib/tlpi-lib
TLPI_LIB  = ${TLPI_DIR}/libtlpi.a

IMGUI_DIR = ../lib/imgui/
IMGUI_SRC = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
IMGUI_SRC += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
#IMGUI_OBJ = $(addsuffix .o, $(basename $(notdir $(IMGUI_SRC))))
IMGUI_OBJ = $(addsuffix .o, $(basename $(IMGUI_SRC)))

IMGUI_BK_DIR = ../lib/imgui/backends/

IMPLOT_DIR = ../lib/implot/
IMPLOT_SRC = $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_demo.cpp $(IMPLOT_DIR)/implot_items.cpp
IMPLOT_OBJ = $(addsuffix .o, $(basename $(IMPLOT_SRC)))

INCLUDES  := -I ./ -I $(LODEPNG_DIR) -I $(TINYTIFF_DIR) -I $(TLPI_DIR) -I $(IMGUI_DIR) -I $(IMGUI_BK_DIR) -I $(IMPLOT_DIR) -I `apr-1-config --includedir` 
LOCAL_SRC_CPP := main.cpp main_cli.cpp imgui_wrapper.cpp opengl_helper.cpp proj.cpp palette.cpp json_helper.cpp dtv.cpp rjpg.cpp graphics.cpp main_menu.cpp viewport.cpp tool_processing.cpp tool_histogram.cpp tool_profile.cpp tool_scale.cpp tool_preferences.cpp help_about.cpp file_library.cpp file_properties.cpp # $(wildcard *.cpp)
LOCAL_SRC_C := # $(wildcard *.c)
SRC        = $(LOCAL_SRC_C) $(LOCAL_SRC_CPP) $(LODEPNG_SRC_C) $(TINYTIFF_SRC_C)
#EXOBJECT   = $(PROJ).o
EXOUTPUT   = $(PROJ)

LOCAL_OBJECTS     = $(LOCAL_SRC_CPP:%.cpp=%.o) $(LOCAL_SRC_C:%.c=%.o)

# Append specific CFLAGS/LDFLAGS
CFLAGS_REL	 += -O3
LDFLAGS_REL	 += -Wl,-s
CFLAGS_DBG	 += -O1 -ggdb -DNDEBUG -pg
LDFLAGS_DBG	 += -pg

DEFAULT_FLAGS=-fPIC -pedantic -Wall -Wextra -Wa,-ahl=$(@:.o=.s)
DEFAULT_LDFLAGS=-lm -ljson-c -lapr-1 -laprutil-1 -lGL `pkg-config --static --libs glfw3`

# First set the flags to their defaults if not supplied externally.
CFLAGS?=$(DEFAULT_FLAGS)
CXXFLAGS?=$(DEFAULT_FLAGS)
LDFLAGS?=$(DEFAULT_LDFLAGS)

CXXFLAGS+=-fPIC -I. -std=c++0x

DEBUG := $(shell grep "^#define CONFIG_DEBUG" config.h)
MACROS := $(shell ./tools/extract_macros.sh config.h)
CFLAGS += $(MACROS)
ifeq ($(DEBUG),)
TARGET	:= RELEASE
CFLAGS	+= $(CFLAGS_REL)
LDFLAGS	+= $(LDFLAGS_REL)
else
TARGET	:= DEBUG
CFLAGS	+= $(CFLAGS_DBG)
LDFLAGS	+= $(LDFLAGS_DBG)
endif

COVERITY_SCAN_TOKEN := $(shell cat ../.secrets.COVERITY_SCAN_TOKEN)

.PHONY: all
.PHONY: clean
.PHONY: tags
.PHONY: lib

all : lib version.h $(EXOUTPUT)

lib : $(LODEPNG_LIB) $(TINYTIFF_LIB)

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
	@$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(EXOUTPUT): $(LOCAL_OBJECTS) $(LODEPNG_LIB) $(TINYTIFF_LIB) $(TLPI_LIB) $(EXOBJECT) $(IMGUI_OBJ) $(IMPLOT_OBJ)
	@echo -e "\n>> Building $@ as target $(TARGET)"
	$(CXX) $(LDFLAGS) $(LOCAL_OBJECTS) $(LODEPNG_LIB) $(TINYTIFF_LIB) $(TLPI_LIB) $(IMGUI_OBJ) $(IMPLOT_OBJ) -o $@ && size $@

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
	  https://scan.coverity.com/builds?project=rodan%2Fthpp

force_look:
	@true
