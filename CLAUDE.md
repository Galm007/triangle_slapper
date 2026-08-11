# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

`triangle_slapper` recreates a target image by repeatedly "slapping" semi-random
triangles onto an initially blank canvas. It is a CPU-only genetic algorithm — no
GPU acceleration, so runs are slow (roughly a minute per iteration on a laptop).

## Build & Run

CMake-based build (uses `-O3`, exports `compile_commands.json`):

```
mkdir build && cd build
cmake ..
cmake --build .
```

The binary is `build/TriangleSlapper`. Run it against a PNG (input image is the
last positional argument):

```
./TriangleSlapper --threads=4 my_image.png
```

There is no test suite, linter config, or CI in this repo. `compile_commands.json`
is generated in `build/` for clangd/IDE tooling.

## Architecture

The program runs a two-level loop (`src/main.c`):

- **Iteration** — each iteration finds ONE best triangle, draws it permanently onto
  `current_img`, and writes the canvas to `output_dir/output_<k>.png`.
- **Generation** (inner loop, `conf.generations` per iteration) — evolves a
  population of candidate triangles toward the best fit.

Population = `threads * thread_tris`. Per generation:
1. All candidate triangles are scored in parallel (`calculate_scores`).
2. Triangles are sorted best-first (`sort_triangles`, bubble sort).
3. The top `best_cutoff` survive; the rest are overwritten with mutated copies of
   survivors (`tris[i] = tris[i % best_cutoff]` then `triangle_mutate`) to form the
   next generation.

If the best score is `<= 0` (the triangle would not improve the image), the whole
iteration restarts (`goto ITERATION_START`) with a fresh random population.

### Scoring (`src/genetic_algorithm.c`)

`triangle_score` measures how much a candidate improves the canvas: it draws the
triangle onto a copy of `current_img`, then compares per-pixel RGB distance to
`target_img` *only within the triangle's bounding box*, against the same region of
the un-drawn canvas. The delta is multiplied by `area * area`, biasing selection
toward larger triangles. Higher score = better.

### Threading (`src/thread_sync.c`)

`ThreadSync` is a reusable worker-pool barrier built on two semaphores. Worker
threads block on `dispatch_sem`; `threadsync_dispatch` posts once per thread to
release them, and `threadsync_wait` collects `finished_sem` posts to join. Each
worker runs `calculate_scores` over its own contiguous slice
(`thread_tris * thread_index`) of the shared triangle/score arrays — no locking is
needed because slices don't overlap. `destroy` flag + a final dispatch unblocks
workers for clean shutdown.

### Rendering (`src/triangle.c`)

Scanline triangle rasterizer (edge/span based) with linear color interpolation
across vertices. When `--no-interpolate` is set, all three vertex colors are equal
so the triangle is flat-shaded. Note: this rasterizer does no bounds clamping, so
triangle vertices must stay within the canvas — mutation/init clamp coordinates to
image dimensions to enforce this.

### Color & I/O (`src/color.c`)

`Color` uses `float` r/g/b channels (allows sub-integer interpolation and mutation
math). `img_load`/`write_img` convert to/from 8-bit RGB via the vendored `stb_image`
libraries in `lib/`. Images are always forced to 3 channels.

### Config (`src/config.c`, `include/config.h`)

All tuning comes from CLI args parsed into a `Config` struct; there is no config
file support. Defaults live in `config_init_w_args`. `--resume-from` continues a
previous run by loading `output_<n>.png`; `--resume-from=-1` (or default detection
via `get_highest_iteration`) picks up from the highest existing `output_*.png` in
`output_dir`. `config_validate` rejects non-positive numeric values and a
non-directory `--output-dir`.

## Conventions

- C, compiled as a single executable from all `src/*.c` (globbed by CMake — new
  `.c` files under `src/` are picked up automatically).
- Headers in `include/`; vendored single-header libs in `lib/`.
- Tabs for indentation; braces on their own line (Allman style).
- `main.c` carries a TODO list of intended features (dynamic mutation, config
  files, opacity, other shapes, OpenCL scoring).
