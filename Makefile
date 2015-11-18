SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -ltbb -lassimp -lboost_program_options -l:libembree.so.2
INCLUDES = -isystem /usr/include/eigen3 -isystem third_party/tinyexr

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
