all: cmake-build/build.ninja
	cmake --build cmake-build

cmake-build/build.ninja:
	cmake -B cmake-build -S . -GNinja

clean:
	rm -rf cmake-build coverage-build coverage-report

# Coverage build with gcov instrumentation
coverage-build/build.ninja:
	cmake -B coverage-build -S . -GNinja \
		-DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage -g -O0" \
		-DCMAKE_C_FLAGS="--coverage -fprofile-arcs -ftest-coverage -g -O0" \
		-DCMAKE_EXE_LINKER_FLAGS="--coverage"

coverage-build: coverage-build/build.ninja
	cmake --build coverage-build

# Generate coverage report (requires lcov)
coverage: coverage-build
	@echo "Running coverage build executables..."
	-./coverage-build/taskHelper || true
	@echo "Generating coverage report..."
	lcov --capture --directory coverage-build --output-file coverage-build/coverage.info --ignore-errors source
	lcov --remove coverage-build/coverage.info '/usr/*' '*/boost/*' --output-file coverage-build/coverage.info --ignore-errors unused
	genhtml coverage-build/coverage.info --output-directory coverage-report
	@echo "Coverage report generated in coverage-report/index.html"

.PHONY: all clean coverage coverage-build
