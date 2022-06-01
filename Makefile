all:
	gcc -O3 -Wall -Werror -lm -Ilib $(wildcard src/*.c) -o build/triangulator

run:
	cd build && ./triangulator

clean:
	rm build/triangulator $(wildcard build/output_*.png)

img:
	sxiv build/output.png
