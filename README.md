# triangle_slapper

Slap triangles on top of each other to recreate an image

The algorithm spawns random triangles, selects the best ones based on their calculated scores, and then creates the next generation by having different variants of the current best triangles. After a certain amount of generations, the best triangle is drawn into the final image and the cycle repeats.

Note: This program does everything in cpu. So it will be slow compared to if it was hardware accelerated in gpu.

## Compiling
This project uses cmake so the build process varies for different operating systems.

For Linux:
```
git clone https://github.com/Nyaaboron/triangle_slapper.git && cd triangle_slapper
mkdir build && cd build
cmake ..
cmake --build .
```

When running the program, it takes one argument for the path to the image that you want to be replicated.
```
./TriangleSlapper my_image.png
```

![](./preview.gif)

^^ Each iteration in this example took my laptop about a minute (Intel core i7 8th gen). It is not realtime.

## Configuration

Configs can be changed by editing the macros in config.h and recompiling.
* THREADS - The number of cpu threads that the program will use.
* MAX_ITERATIONS - The program will automatically stop after a certain number of iterations. Each iteration will produce a single triangle that will be drawn on the output image.
* GENERATIONS - The number of generations per iteration.
* POPULATION - The number of triangles per generation.
* BEST_CUTOFF - The number of triangles that survive per generation. The next generation's population will be based on the current generation's survivors.
* MUTATION_AMOUNT_POS - Maximum amount of change in a triangle's position per generation.
* MUTATION_AMOUNT_CLR - Maximum amount of change in a triangle's color per generation.
* INTERPOLATED_TRIANGLES - Use triangles with interpolated colors. Comment out this macro to use non-interpolated colors.

For small images, it's better for MUTATION_AMOUNT_POS and MUTATION_AMOUNT_CLR to have smaller values.

Higher values for GENERATIONS and POPULATION will produce better results but will take significantly longer time to finish an iteration.
