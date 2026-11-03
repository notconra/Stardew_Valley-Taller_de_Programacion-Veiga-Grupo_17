PRECOMMIT = python3 -m pre_commit
JOBS ?= 3

deps:   ## instala todo lo que necesita el proyecto (pide sudo)
	sudo apt-get update
	sudo apt-get install -y build-essential cmake pkg-config git libgtest-dev ccache lld \
		libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev qt6-base-dev qt6-base-dev-tools \
		python3-pip

hooks:   ## instala pre-commit y activa los hooks
	python3 -m pip install --user pre-commit || \
		python3 -m pip install --user --break-system-packages pre-commit
	$(PRECOMMIT) install --hook-type pre-commit --hook-type commit-msg

build:   ## compila server y client (make build JOBS=4 para compilar en paralelo con mas hilos)
	cmake -S . -B build
	cmake --build build -j$(JOBS) --target server client

test:   ## compila y corre los tests
	cmake -S . -B build
	cmake --build build -j$(JOBS) --target tests
	./build/tests

format:   ## da formato al codigo (misma version de clang-format que los hooks)
	-$(PRECOMMIT) run clang-format --all-files

lint:   ## revisa el estilo con cpplint (usa CPPLINT.cfg)
	$(PRECOMMIT) run cpplint --all-files

check:   ## corre TODOS los hooks sobre todo el repo
	$(PRECOMMIT) run --all-files

clean:
	rm -rf build build-tsan
	find . -type f \( -name "*.o" -o -name "*.tsan.o" \) -delete
	rm -f client server client-tsan server-tsan

.PHONY: deps hooks build test format lint check clean
