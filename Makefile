path-tracer: *.cc
	mkdir -p bin
	g++ *.cc -lIlmImf -lHalf -std=c++11 -o bin/path-tracer

clean:
	rm -r bin
