# triangle_slapper

Slap triangles on top of each other to recreate an image

The algorithm spawns random triangles, selects the best ones based on their calculated scores, and then creates the next generation by having different variants of the current best triangles. After a certain amount of generations, the best triangle is drawn into the final image and the cycle repeats.

Note: This program does everything in cpu. So it will be slow compared to if it was hardware accelerated in gpu.

## Preview
![](./preview.gif)

```
./TriangleSlapper --threads=4 my_image.png
```

^^ Each iteration in this example took my laptop about a minute to finish (Intel core i7 8th gen). It is not realtime.

## Compiling
This project uses cmake so the build process varies for different operating systems.

For Linux:
```
git clone https://github.com/Nyaaboron/triangle_slapper.git && cd triangle_slapper
mkdir build && cd build
cmake ..
cmake --build .
```

## Configuration

Default configs can be overriden using command line arguments.
* `--threads=8` - The number of cpu threads that the program will use.
* `--thread-tris=50` - The amount of triangles that each thread will process per generation.
* `--max-iter=1000` - The program will automatically stop after a certain number of iterations. Each iteration will produce a single triangle that will be drawn on the output image.
* `--generations=100` - The number of generations per iteration.
* `--cutoff=20` - The number of triangles that survive per generation. The next generation's population will be based on the current generation's survivors.
* `--max-pos-mut=30` - Maximum amount of change in a triangle's position per generation.
* `--max-clr-mut=20` - Maximum amount of change in a triangle's color per generation.
* `--no-interpolate` - Do not use triangles with interpolated colors.
* `--output-dir=.` - Where to write the output images.

Higher values for `--generations` and `--thread-tris` will produce better results but it will take a longer time to finish an iteration.
