UNAME := $(shell uname)

SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -lIlmImf -lHalf -ltbb -lassimp -lboost_program_options
INCLUDES = -isystem /usr/local/include/eigen3 -isystem /usr/include/eigen3 \
           -isystem /usr/local/include/OpenEXR -isystem /usr/include/OpenEXR

# Choose specific Embree v2 library based on filename. Filename depends on OS.
ifeq ($(UNAME), Linux)
	LDLIBS += -l:libembree.so.2
endif

ifeq ($(UNAME), Darwin)
	LDLIBS += -lembree.2
endif

ifeq ($(strip $(CXX)),clang++)
	# Clang compiler: enable extra warnings

	WARN = -Werror -Weverything -Wno-c++98-compat -Wno-padded -Wno-float-equal \
			   -Wno-exit-time-destructors

	CXXFLAGS = $(WARN) -std=c++11 -DNDEBUG -O3 -flto
	CXXFLAGS_DEBUG = $(WARN) -std=c++11 -DDEBUG -O0 -g
	CXXFLAGS_GENPROF = $(CXXFLAGS)
	CXXFLAGS_USEPROF = $(CXXFLAGS)

else
	# Anything else: lowest common denominator

	WARN = -Werror -Wall

	CXXFLAGS = $(WARN) -std=c++11 -DNDEBUG -O3
	CXXFLAGS_DEBUG = $(WARN) -std=c++11 -DDEBUG -O0 -g
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
