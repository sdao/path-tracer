SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -lIlmImf -lHalf -ltbb -lassimp

# icpc or c++ or g++ or clang++
CXX = icpc
CXXFLAGS_DEBUG = -std=c++11 -O0 -g

ifeq ($(strip $(CXX)),icpc)
	CXXFLAGS = -std=c++11 -fast
	WARN = -Werror -Wall
	PROFGEN = -prof-gen -prof-dir=prof
	PROFUSE = -prof-use -prof-dir=prof
else
	CXXFLAGS = -std=c++11 -O3 -flto
	WARN = -Werror -Weverything -Wno-c++98-compat -Wno-padded -Wno-float-equal
endif

all: path-tracer

# Standard targets
path-tracer: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(CXXFLAGS) $(WARN) -o bin/path-tracer

debug: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(CXXFLAGS_DEBUG) $(WARN) -o bin/path-tracer

profile: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(CXXFLAGS) $(WARN) $(PROFGEN) -o bin/path-tracer

useprofile: $(SOURCES)
	mkdir -p bin
	$(CXX) $(SOURCES) $(LDLIBS) $(CXXFLAGS) $(WARN) $(PROFUSE) -o bin/path-tracer

clean:
	rm -rf bin

# Xcode-specific targets
xcode: debug
xcodeclean: clean
