build:   ## compila server y client
	cmake -S . -B build
	cmake --build build -j2

test: build   ## corre los tests
	ctest --test-dir build --output-on-failure

clean:
	rm -rf build build-tsan

.PHONY: deps build test tsan clean
