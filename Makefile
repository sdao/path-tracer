SOURCES = *.cc materials/*.cc geoms/*.cc

path-tracer: $(SOURCES)
	mkdir -p bin
	g++ $(SOURCES) -lIlmImf -lHalf -ltbb -std=c++11 -O3 -o bin/path-tracer

debug: $(SOURCES)
	mkdir -p bin
	g++ $(SOURCES) -lIlmImf -lHalf -ltbb -std=c++11 -g -o bin/path-tracer

clean:
	rm -r bin
