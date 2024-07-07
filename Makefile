.PHONY: build_debug
build_debug:
	mkdir -p build/debug
	cmake -E chdir build/debug cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../..
	cmake --build build/debug

.PHONY: build_release
build_release:
	mkdir -p build/release
	cmake -E chdir build/release cmake -DCMAKE_BUILD_TYPE=Release ../..
	cmake --build build/release

.PHONY: build_stm32
build_stm32:
	test -n "$(MCU)"
	mkdir -p build/$(MCU)
	cmake -E chdir build/$(MCU) cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/opt/stm32/toolchain/$(MCU).cmake ../..
	cmake --build build/$(MCU)

.PHONY: test
test:
	cd build/debug && ctest --output-on-failure

.PHONY: install
install:
	cmake --build build/release --target install

.PHONY: install_stm32
install_stm32:
	test -n "$(MCU)"
	cmake --build build/$(MCU) --target install
