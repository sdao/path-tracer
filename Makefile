SOURCES = *.cc materials/*.cc geoms/*.cc
LDLIBS = -lIlmImf -lHalf -ltbb -lassimp

# icpc or c++ or g++ or clang++
CXX = icpc
CXXFLAGS_DEBUG = -std=c++11 -O0 -g

ifeq ($(strip $(CXX)),icpc)
	CXXFLAGS = -std=c++11 -O3 -no-prec-div -no-prec-sqrt -ipo -xhost -parallel
	WARN = -Werror -Wall
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

clean:
	rm -rf bin

# Xcode-specific targets
xcode: debug
xcodeclean: clean
