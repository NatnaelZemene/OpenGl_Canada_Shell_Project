# Canada Flag OpenGL Project - Presentation Guide

## Quick Run (Easiest)

From project folder, run this one command in PowerShell:

```powershell
c
```

If you get a missing DLL error, copy once and run again:

```powershell
Copy-Item .\bin\freeglut.dll . -Force
g++ main.cpp -o app.exe -Iinclude -Llib -lfreeglut -lopengl32 -lglu32; .\app.exe
```

### **Execution**

Run the executable:

```bash
.\canda_flag.exe
```

This document explains the full `main.cpp` implementation in a presentation-friendly way.

## 1) Project Goal

The program renders the National Flag of Canada using FreeGLUT + legacy OpenGL immediate mode.

Main targets achieved:

- Correct flag structure (red-white-red vertical layout).
- Correct 2:1 overall aspect ratio.
- Maple leaf generated from SVG path data, then normalized to fit the center white area.
- Window resize handling that avoids distortion.

## 2) High-Level Rendering Pipeline

The code works in this order:

1. Parse SVG path string into geometric points (`parseSvgPath`).
2. Select leaf subpath and normalize it into OpenGL coordinates (`buildNormalizedLeafFromSvg`).
3. During rendering:

- Draw red side bars and white center (`drawFlagBackground`).
- Draw maple leaf using `GL_TRIANGLE_FAN` (`drawMapleLeaf`).

4. On window resize:

- Keep viewport letterboxed/pillarboxed to preserve 2:1 aspect (`reshape`).

## 3) Include Files and Why They Are Used

- `#include <GL/freeglut.h>`: windowing + OpenGL context + callbacks.
- `#include <algorithm>`: `std::min`, `std::max`, `std::max_element`.
- `#include <cctype>`: `std::isspace` while parsing text.
- `#include <cmath>`: math support (general-purpose).
- `#include <cstdlib>`: `std::strtod` for float parsing.
- `#include <string>`: SVG path string storage.
- `#include <utility>`: `std::pair<float,float>`.
- `#include <vector>`: dynamic point arrays.

## 4) Data Structures and Constants

### `struct Point { float x; float y; };`

A lightweight type used during SVG parsing.

### `static std::vector<std::pair<float, float>> gLeafPoints;`

Stores final normalized leaf coordinates used by OpenGL drawing.

### `kCurveSamples = 14`

How many line segments approximate each SVG cubic Bezier segment.

- Higher value: smoother curve, more vertices.
- Lower value: less smooth but faster.

### `kLeafFit = 0.86f`

Controls how large the leaf appears inside the center white square.

- `1.0` means touching boundaries more closely.
- `0.86` leaves margin around leaf for visual correctness.

## 5) Function-by-Function Explanation

## `float cubic(float p0, float p1, float p2, float p3, float t)`

Evaluates cubic Bezier interpolation at parameter `t` in range [0,1].

Formula:
`B(t) = (1-t)^3*p0 + 3(1-t)^2*t*p1 + 3(1-t)*t^2*p2 + t^3*p3`

Used to convert SVG `C/c` curve commands into a polyline.

## `skipSeparators(const std::string& s, size_t& i)`

Skips whitespace and commas while reading the SVG path string.

## `readFloat(const std::string& s, size_t& i, float& out)`

Reads one numeric value from current string index using `strtod`.
Returns `false` if there is no valid number.

## `parseSvgPath(const std::string& d)`

Parses SVG path commands and returns a list of subpaths.

Supported commands:

- `M/m`: move to
- `L/l`: line to
- `H/h`: horizontal line
- `V/v`: vertical line
- `C/c`: cubic Bezier
- `Z/z`: close path

Important detail:

- Relative commands (`m,l,h,v,c`) are added to current point.
- Absolute commands (`M,L,H,V,C`) replace coordinates directly.

Output type:
`std::vector<std::vector<Point>>`

- Outer vector = multiple subpaths
- Inner vector = points for one subpath

## `buildNormalizedLeafFromSvg()`

Core geometry preparation step.

What it does:

1. Stores full SVG path text in raw-string literal.
2. Parses all subpaths.
3. Picks the most detailed subpath (largest point count), which corresponds to maple leaf.
4. Computes bounding box: `minX, maxX, minY, maxY`.
5. Computes center and scale so leaf fits into `kLeafFit` box.
6. Converts SVG space to OpenGL space:

- Translate to center around origin (0,0).
- Uniform scale preserving leaf proportions.
- Invert Y (`ny = -(...)`) because SVG Y grows downward while OpenGL Y grows upward.

7. Stores final coordinates into `gLeafPoints`.

This is the reason the leaf remains geometrically faithful while fitting the white area.

## `drawFlagBackground()`

Draws three vertical regions using `GL_QUADS`:

- Left red bar: x from `-1.0` to `-0.5`
- White center: x from `-0.5` to `0.5`
- Right red bar: x from `0.5` to `1.0`

Y range is `-0.5` to `0.5`, giving total flag ratio 2:1.

## `drawMapleLeaf()`

Draws leaf in red using `GL_TRIANGLE_FAN`.

Why `GL_TRIANGLE_FAN` here:

- User required this approach.
- It forms triangles from center `(0,0)` to each pair of neighboring boundary points.

Extra closure:

- Re-submits first boundary point at end to close fan contour cleanly.

## `display()`

Per-frame callback:

1. `glClear(GL_COLOR_BUFFER_BIT)`
2. Draw background
3. Draw leaf
4. `glutSwapBuffers()` for double buffering

## `reshape(int width, int height)`

Keeps rendering undistorted during resize.

Technique:

- Target aspect ratio = `2.0`.
- If window too wide: reduce viewport width and center horizontally.
- If window too tall: reduce viewport height and center vertically.

Then projection is set to:
`glOrtho(-1.0, 1.0, -0.5, 0.5, -1.0, 1.0)`

This guarantees consistent world coordinates and preserves geometry.

## `main()`

Initialization sequence:

1. `glutInit`
2. Double-buffered RGB mode
3. Create 1920x960 window (2:1)
4. Build leaf points from SVG
5. Register callbacks (`display`, `reshape`)
6. Enter event loop (`glutMainLoop`)

## 6) Coordinate Systems Summary

There are 3 spaces involved:

1. SVG source space:

- Original large coordinate values from path.
- Y axis points down.

2. Normalized model space:

- Leaf transformed to around origin and scaled to fit white center.
- Y corrected to OpenGL direction.

3. OpenGL world space (`glOrtho`):

- X in [-1,1], Y in [-0.5,0.5].
- Flag occupies full projection box.

## 7) Why the Leaf Looks Accurate

- The shape is not hand-approximated; it is derived from the provided SVG path data.
- Cubic curves are sampled into many points.
- Uniform scaling avoids non-proportional stretching.
- Y inversion fixes orientation mismatch.
- Centering by bounding box keeps leaf aligned in white region.

## 8) Build and Run Commands

Use this command in project root:

```powershell
g++ main.cpp -o app.exe -Iinclude -Llib -lfreeglut -lopengl32 -lglu32
```

Then run:

```powershell
.\app.exe
```

If `app.exe` is locked, close/kill old process then rebuild:

```powershell
Stop-Process -Name "app" -Force -ErrorAction SilentlyContinue
g++ main.cpp -o app.exe -Iinclude -Llib -lfreeglut -lopengl32 -lglu32
.\app.exe
```

## 9) Common Questions (Teacher Viva) and Suggested Answers

Q1) Why use `glOrtho(-1,1,-0.5,0.5,-1,1)`?

- It directly maps to flag ratio 2:1 in world coordinates, making layout math simple and exact.

Q2) Why not draw leaf manually with fixed points?

- SVG source provides a high-fidelity reference shape; parsing avoids losing detail and improves accuracy.

Q3) Why invert Y during normalization?

- SVG Y-axis increases downward, OpenGL Y-axis increases upward.

Q4) Why use `GL_TRIANGLE_FAN` for leaf?

- It creates a filled shape from center + boundary points and follows the project requirement.

Q5) How do you keep shape from stretching on resize?

- `reshape()` computes a centered viewport with fixed 2:1 aspect before applying orthographic projection.

Q6) What is the role of `kCurveSamples`?

- It controls curve tessellation quality. More samples produce smoother Bezier approximation.

Q7) Why choose the largest subpath as leaf?

- The SVG contains multiple subpaths (including bars). The leaf contour has the highest point density after curve expansion.

Q8) Why double buffering?

- `GLUT_DOUBLE` + `glutSwapBuffers()` prevents flicker and tearing during redraw.

Q9) What are limitations of this legacy approach?

- Uses immediate mode (`glBegin/glEnd`), which is deprecated in modern OpenGL. Modern approach uses VBO/VAO/shaders.

Q10) If leaf appears too large/small, what do you change?

- Adjust `kLeafFit` constant only; geometry and proportions remain intact.

## 10) Quick Slide Outline for Presentation

1. Problem statement and expected flag correctness
2. Architecture (Parser -> Normalize -> Render)
3. SVG parsing strategy and command support
4. Geometry normalization and Y inversion
5. OpenGL rendering steps and projection setup
6. Resize handling for aspect-ratio safety
7. Demo + Q&A

## 11) Optional Improvements (if asked)

- Replace immediate mode with VBO/VAO + shader pipeline.
- Cache tessellated curve points to file for faster startup.
- Add anti-aliasing for smoother leaf edges.
- Add unit tests for parser command correctness.

## 12) One-Minute Summary Script

"This project renders the Canadian flag with exact proportions using FreeGLUT. The background is drawn with three quads in a 2:1 orthographic projection. The maple leaf is not guessed manually; it is built by parsing SVG path data, including cubic Bezier commands. The parser converts curves into sampled points, then normalizes and centers the shape into the white region while preserving proportions. On window resize, letterboxing logic preserves aspect ratio so the flag never stretches. This design balances visual accuracy, deterministic geometry, and clear educational code structure."
