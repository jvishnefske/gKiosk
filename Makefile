all: cmake-build/build.ninja example-0
	cmake --build cmake-build 
cmake-build/build.ninja:
	cmake -B cmake-build -S . -GNinja 
clean:
	rm -rf cmake-build
