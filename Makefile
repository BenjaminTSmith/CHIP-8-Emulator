ifndef VERBOSE
.SILENT:
endif

NC=\033[0m
RED=\033[0;31m
GREEN=\033[0;32m

.PHONY: build

build :
	mkdir -p build
	echo "[${RED}0%${NC}] Compiling Source Files."
	gcc -Wall -o build/main src/*.c -I/usr/include/SDL2 -lSDL2
	echo "[${GREEN}100%${NC}] Finished Compiling."

clean :
	rm build/main
