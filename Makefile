SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -ltbb -lassimp -lboost_program_options -l:libembree.so.2
INCLUDES = -isystem /usr/include/eigen3 -isystem third_party/tinyexr

CXXFLAGS = $(WARN) -std=c++11 -DNDEBUG -O3
CXXFLAGS_DEBUG = $(WARN) -std=c++11 -DDEBUG -O0 -g=
WARN = -Werror -Wall

all: path-tracer

# Standard targets
path-tracer: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(INCLUDES) $(LDLIBS) $(CXXFLAGS) -o bin/path-tracer

debug: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(INCLUDES) $(LDLIBS) $(CXXFLAGS_DEBUG) -o bin/path-tracer

clean:
	rm -rf bin
