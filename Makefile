deps:   ## instala todo lo que necesita el proyecto (pide sudo)
	sudo apt-get update
	sudo apt-get install -y build-essential cmake git libgtest-dev \
		libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev qtbase5-dev

build:   ## compila server y client
	cmake -S . -B build
	cmake --build build -j2

test: build   ## corre los tests
	ctest --test-dir build --output-on-failure

clean:
	rm -rf build build-tsan

.PHONY: deps build test tsan clean
