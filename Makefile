SOURCES = *.cc materials/*.cc geoms/*.cc
LIBS = -lIlmImf -lHalf -ltbb -lassimp
WARN = -Werror -Weverything -Wno-c++98-compat -Wno-padded

all: path-tracer

# Standard targets
path-tracer: $(SOURCES)
	mkdir -p bin
	g++ $(SOURCES) $(LIBS) -std=c++11 -O3 -flto $(WARN) -o bin/path-tracer

debug: $(SOURCES)
	mkdir -p bin
	g++ $(SOURCES) $(LIBS) -std=c++11 -O0 -g $(WARN) -o bin/path-tracer

clean:
	rm -rf bin

# Xcode-specific targets
xcode: debug
xcodeclean: clean
