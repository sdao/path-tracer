SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -lIlmImf -lHalf -ltbb -lassimp
INCLUDES = -isystem /usr/local/include/eigen3 -isystem /usr/include/eigen3 \
           -isystem /usr/include/OpenEXR/

ifeq ($(strip $(CXX)),icpc)
	WARN = -Werror -Wall -Wcheck
	CXXFLAGS = $(WARN) -std=c++11 -DNDEBUG -fast -parallel
	CXXFLAGS_DEBUG = $(WARN) -std=c++11 -DDEBUG -O0 -g

	# Enable profile guided optimization for Intel compiler
	CXXFLAGS_GENPROF = $(CXXFLAGS) -prof-gen -prof-dir=prof
	CXXFLAGS_USEPROF = $(CXXFLAGS) -prof-use -prof-dir=prof
else
	WARN = -Werror -Weverything -Wno-c++98-compat -Wno-padded -Wno-float-equal
	CXXFLAGS = $(WARN) -std=c++11 -DNDEBUG -O3 -flto
	CXXFLAGS_DEBUG = $(WARN) -std=c++11 -DDEBUG -O0 -g

	# For the moment, skip profile guided optimization on Clang/GCC/etc
	CXXFLAGS_GENPROF = $(CXXFLAGS)
	CXXFLAGS_USEPROF = $(CXXFLAGS)
endif

all: path-tracer

# Standard targets
path-tracer: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(INCLUDES) $(CXXFLAGS) -o bin/path-tracer

debug: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(INCLUDES) $(CXXFLAGS_DEBUG) -o bin/path-tracer

profile: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(INCLUDES) $(CXXFLAGS_GENPROF) -o bin/path-tracer

useprofile: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(INCLUDES) $(CXXFLAGS_USEPROF) -o bin/path-tracer

clean:
	rm -rf bin

# Xcode-specific targets
xcode: debug
xcodeclean: clean
