path-tracer: *.cc
	mkdir -p bin
	g++ *.cc -lIlmImf -lHalf -ltbb -std=c++11 -O3 -o bin/path-tracer

debug: *.cc
	mkdir -p bin
	g++ *.cc -lIlmImf -lHalf -ltbb -std=c++11 -g -o bin/path-tracer

clean:
	rm -r bin
