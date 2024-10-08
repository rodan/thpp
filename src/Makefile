
PROJ = thpp

LODEPNG_DIR = ../lib/lodepng
LODEPNG_SRC = ${LODEPNG_DIR}/lodepng.cpp
LODEPNG_OBJ = $(patsubst ${LODEPNG_DIR}/%.cpp,build/lib/lodepng/%.o,${LODEPNG_SRC})

TINYTIFF_DIR = ../lib/TinyTIFF/src
TINYTIFF_SRC = ${TINYTIFF_DIR}/tinytiffreader.c ${TINYTIFF_DIR}/tinytiff_ctools_internal.c
TINYTIFF_OBJ = $(patsubst ${TINYTIFF_DIR}/%.c,build/lib/TinyTIFF/%.o,${TINYTIFF_SRC})

TLPI_DIR  = ../lib/tlpi-lib
TLPI_SRC  = ${TLPI_DIR}/error_functions.cpp ${TLPI_DIR}/get_num.cpp
TLPI_OBJ  = $(patsubst ${TLPI_DIR}/%.cpp,build/lib/tlpi/%.o,${TLPI_SRC})

EXIFTOOL_DIR = ../lib/cpp_exiftool
EXIFTOOL_SRC = ${EXIFTOOL_DIR}/src/ExifTool.cpp ${EXIFTOOL_DIR}/src/ExifToolPipe.cpp ${EXIFTOOL_DIR}/src/TagInfo.cpp
#EXIFTOOL_OBJ = $(addsuffix .o, $(basename $(EXIFTOOL_SRC)))
EXIFTOOL_OBJ = $(patsubst ${EXIFTOOL_DIR}/src/%.cpp,build/lib/cpp_exiftool/%.o,${EXIFTOOL_SRC})

IMGUI_DIR = ../lib/imgui/
IMGUI_SRC = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
IMGUI_SRC += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
#IMGUI_OBJ = $(addsuffix .o, $(basename $(IMGUI_SRC)))
IMGUI_OBJ = $(patsubst ${IMGUI_DIR}/%.cpp,build/lib/imgui/%.o,${IMGUI_SRC})

IMGUI_BK_DIR = ../lib/imgui/backends/

IMPLOT_DIR = ../lib/implot/
IMPLOT_SRC = $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_demo.cpp $(IMPLOT_DIR)/implot_items.cpp
#IMPLOT_OBJ = $(addsuffix .o, $(basename $(IMPLOT_SRC)))
IMPLOT_OBJ = $(patsubst ${IMPLOT_DIR}/%.cpp,build/lib/implot/%.o,${IMPLOT_SRC})

LIBS_OBJ = $(LODEPNG_OBJ) $(TINYTIFF_OBJ) $(TLPI_OBJ) $(EXIFTOOL_OBJ) $(IMGUI_OBJ) $(IMPLOT_OBJ)

INCLUDES  := -I ./ -I $(LODEPNG_DIR) -I $(TINYTIFF_DIR) -I $(TLPI_DIR) -I $(IMGUI_DIR) -I $(IMGUI_BK_DIR) -I $(IMPLOT_DIR) -I $(EXIFTOOL_DIR)/inc/ -I /usr/local/include/
LOCAL_SRC_CPP := main.cpp main_cli.cpp imgui_wrapper.cpp opengl_helper.cpp proj.cpp palette.cpp dtv.cpp rjpg.cpp graphics.cpp main_menu.cpp viewport.cpp tool_processing.cpp tool_histogram.cpp tool_profile.cpp tool_scale.cpp tool_export.cpp tool_preferences.cpp help_about.cpp file_library.cpp file_properties.cpp # $(wildcard *.cpp)
LOCAL_SRC_C := # $(wildcard *.c)
SRC        = $(LOCAL_SRC_C) $(LOCAL_SRC_CPP) $(LODEPNG_SRC) $(TINYTIFF_SRC) $(TLPI_SRC)
#EXOBJECT   = $(PROJ).o
EXOUTPUT   = $(PROJ)

#LOCAL_OBJECTS     = $(LOCAL_SRC_CPP:%.cpp=%.o) $(LOCAL_SRC_C:%.c=%.o)
LOCAL_OBJECTS     = $(patsubst %.cpp,build/%.o,${LOCAL_SRC_CPP})

# Append specific CFLAGS/LDFLAGS
CFLAGS_REL	 += -O3
LDFLAGS_REL	 += -Wl,-s
CFLAGS_DBG	 += -O1 -ggdb -DNDEBUG # -pg
LDFLAGS_DBG	 += # -pg

MANDATORY_CFLAGS=-fPIC -pedantic -Wall -Wextra -Wno-format-truncation -fopenmp # -Wa,-ahl=$(@:.o=.s)
MANDATORY_LDFLAGS=-lm -lGL `pkg-config --static --libs glfw3` -fopenmp

CXXFLAGS+=-std=c++17

DEBUG := $(shell grep "^#define CONFIG_DEBUG" config.h)
MACROS := $(shell ./tools/extract_macros.sh config.h)
CFLAGS += $(MACROS)
ifeq ($(DEBUG),)
TARGET	:= RELEASE
TARGET_CFLAGS	+= $(MANDATORY_CFLAGS) $(CFLAGS_REL)
TARGET_LDFLAGS	+= $(MANDATORY_LDFLAGS) $(LDFLAGS_REL)
else
TARGET	:= DEBUG
TARGET_CFLAGS	+= $(MANDATORY_CFLAGS) $(CFLAGS_DBG)
TARGET_LDFLAGS	+= $(MANDATORY_LDFLAGS) $(LDFLAGS_DBG)
endif

COVERITY_SCAN_TOKEN := $(shell cat ../.secrets.COVERITY_SCAN_TOKEN 2>/dev/null)

.PHONY: all
.PHONY: clean
.PHONY: tags
.PHONY: lib

all : libs version.h $(EXOUTPUT)
libs : $(LIBS_OBJ)
$(EXOUTPUT) : libs version.h

version.h: $(SRC)
	@./tools/setver.sh

build/lib/tlpi/%.o: ${TLPI_DIR}/%.cpp
	@bash ../lib/tlpi-lib/Build_ename.sh > ../lib/tlpi-lib/ename.c.inc
	@mkdir -p build/lib/tlpi
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -Wno-write-strings -c $< -o $@

build/lib/imgui/%.o: ${IMGUI_DIR}/%.cpp
	@mkdir -p build/lib/imgui
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -c $< -o $@

build/lib/imgui/backends/%.o: ${IMGUI_DIR}/backends/%.cpp
	@mkdir -p build/lib/imgui/backends
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -c $< -o $@

build/lib/implot/%.o: ${IMPLOT_DIR}/%.cpp
	@mkdir -p build/lib/implot
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -c $< -o $@

build/lib/lodepng/%.o: ${LODEPNG_DIR}/%.cpp
	@mkdir -p build/lib/lodepng
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -c $< -o $@

build/lib/TinyTIFF/%.o: ${TINYTIFF_DIR}/%.c
	@mkdir -p build/lib/TinyTIFF
	@echo -e " * $(@F)"
	@$(CC) $(INCLUDES) $(TARGET_CFLAGS) -Wno-unused-parameter -Wno-unused-function -c $< -o $@

build/lib/cpp_exiftool/%.o: ${EXIFTOOL_DIR}/src/%.cpp
	@mkdir -p build/lib/cpp_exiftool
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -Wno-unused-parameter -Wno-unused-result -c $< -o $@

build/%.o: %.cpp
	@mkdir -p build
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@echo -e " * $(@F)"
	@$(CXX) $(INCLUDES) $(TARGET_CFLAGS) -c $< -o $@

$(EXOUTPUT): $(LOCAL_OBJECTS) $(TLPI_OBJ) $(LODEPNG_OBJ) $(TINYTIFF_OBJ) $(TLPI_LIB) $(EXOBJECT) $(IMGUI_OBJ) $(EXIFTOOL_OBJ) $(IMPLOT_OBJ)
	@echo -e "\n>> Building $@ as target $(TARGET)"
	$(CXX) $(TARGET_LDFLAGS) $(LOCAL_OBJECTS) $(TLPI_OBJ) $(LODEPNG_OBJ) $(TINYTIFF_OBJ) $(TLPI_LIB) $(IMGUI_OBJ) $(EXIFTOOL_OBJ) $(IMPLOT_OBJ) -o $@ && size $@

clean :
	@${RM} build/*.o build/*.s ${PROJ}
	@${RM} -rf cov-int

clean-lib:
	@${RM} -rf build/lib

tags: $(SRCS)
	@echo "Generating tags .."
	@exuberant-ctags -R

cppcheck: $(PROJ)
	@cppcheck --quiet --force --enable=warning --enable=performance --enable=information --enable=performance --enable=portability -I ./ ./

scan-build: clean
	@scan-build make

coverity: clean clean-lib
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
