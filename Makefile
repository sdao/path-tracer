SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -lIlmImf -lHalf -ltbb -lassimp
INCLUDES = -isystem /usr/local/include/eigen3 -isystem /usr/include/eigen3

ifeq ($(strip $(CXX)),icpc)
	# Intel C++ compiler: enable profile-guided optimization.

	WARN = -Werror -Wall -Wcheck

	CXXFLAGS = $(WARN) -std=c++11 -DNDEBUG -fast -parallel
	CXXFLAGS_DEBUG = $(WARN) -std=c++11 -DDEBUG -O0 -g
	CXXFLAGS_GENPROF = $(CXXFLAGS) -prof-gen -prof-dir=prof
	CXXFLAGS_USEPROF = $(CXXFLAGS) -prof-use -prof-dir=prof

else ifeq ($(strip $(CXX)),clang++)
	# Clang compiler: enable extra warnings.

	WARN = -Werror -Weverything -Wno-c++98-compat -Wno-padded -Wno-float-equal

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
