# 🍁 National Flag of Canada — OpenGL Project Report

**Course:** Computer Graphics  
**Project:** Flag & Logo Rendering with Transformations  
**Team Members:** FisihaM23 · meronkifle63-hub · edom-jpg  
**Language:** C++ with OpenGL (FreeGLUT + Legacy Immediate Mode)

---

## 📌 Project Overview

Our team was assigned the **National Flag of Canada**. The flag consists of two vertical red bars on the left and right, a white center panel, and a red maple leaf in the middle. Our goal was to render this flag accurately in OpenGL, apply the required transformations, implement correct real-world colors, and add a meaningful bonus feature.

We divided the work into three equal parts, each team member owning a distinct layer of the implementation — from raw geometry parsing, to rendering, to the full OpenGL setup and animation.

---

## 👥 Team Contributions

### 🔵 FisihaM23 — SVG Path Parser & Maple Leaf Builder

**What I did:**

My responsibility was to produce the accurate maple leaf geometry. Instead of manually guessing coordinates, I used the **official Canadian flag SVG path data** — the same vector data used in the real flag specification. This gave us a mathematically precise leaf shape.

**How I did it:**

I wrote a complete SVG path parser from scratch that supports all the commands needed:

| SVG Command | Meaning |
|---|---|
| `M / m` | Move to (absolute / relative) |
| `L / l` | Line to |
| `H / h` | Horizontal line |
| `V / v` | Vertical line |
| `C / c` | Cubic Bezier curve |
| `Z / z` | Close path |

The most important part was handling **cubic Bezier curves** (`C/c` commands). The maple leaf outline is made of smooth curves, not straight lines. I implemented the standard cubic Bezier formula:

```
B(t) = (1-t)³·P0 + 3(1-t)²t·P1 + 3(1-t)t²·P2 + t³·P3
```

I sampled each curve into **40 line segments** (`kCurveSamples = 14` for the flag, 40 for the shell) to get a smooth, high-resolution outline.

After parsing, I normalized the leaf into OpenGL coordinate space:
- Computed the bounding box of all parsed points
- Centered the shape around the origin `(0, 0)`
- Scaled it uniformly to fit inside the white center panel
- **Flipped the Y axis** because SVG Y grows downward while OpenGL Y grows upward

The result is stored in `gLeafPoints` — a list of normalized `(x, y)` pairs ready for rendering.

**Key functions I wrote:**
- `cubic()` — evaluates a cubic Bezier at parameter t
- `skipSeparators()` — skips whitespace and commas in the SVG string
- `readFloat()` — reads one float value from the path string
- `parseSvgPath()` — full SVG path parser returning subpaths
- `buildNormalizedLeafFromSvg()` — picks the leaf subpath and normalizes it to OpenGL space

**Challenge I faced:**

The SVG coordinate system has Y pointing down, but OpenGL has Y pointing up. Forgetting to flip Y caused the leaf to appear upside down. The fix was simple — negate Y during normalization: `ny = -(p.y - srcCY) * scale`.

---

### 🟢 meronkifle63-hub — Flag Rendering & Waving Animation (Bonus Feature)

**What I did:**

My responsibility was to draw the actual flag on screen and implement the **bonus waving animation** — making the flag ripple like it is blowing in the wind.

**How I did it:**

**Flag Background:**

Instead of drawing three static rectangles, I split the flag into **80 thin vertical strips** from left (`x = -1`) to right (`x = 1`). Each strip is drawn as a `GL_QUADS` primitive. The color of each strip is determined by its horizontal position:

```
x in [-1.0, -0.5]  →  Canadian Red
x in [-0.5,  0.5]  →  White
x in [ 0.5,  1.0]  →  Canadian Red
```

**Wave Effect (Bonus Feature):**

The wave is a **vertical sine displacement** applied to each strip. The key insight is that a real flag is attached to a pole on the left — so the left edge doesn't move, and the wave amplitude grows toward the right (free) edge.

```cpp
float waveOffset(float x) {
    float t        = (x + 1.0f) * 0.5f;   // normalize x to [0, 1]
    float envelope = t * t;                 // zero at pole, max at free edge
    return envelope * 0.07f * sin(waveTime * 3.0f + t * 6.28f);
}
```

- `t` maps x from `[-1, 1]` to `[0, 1]`
- `envelope = t²` pins the left edge and grows the amplitude rightward
- `sin(waveTime * 3.0f + t * 6.28f)` creates the traveling wave shape
- `waveTime` increases each frame to make the wave move forward

**Maple Leaf Wave:**

The leaf also waves with the flag — it is not drawn separately. Each leaf vertex gets the same `waveOffset()` applied based on its x position, so the leaf deforms naturally with the fabric.

**Key functions I wrote:**
- `waveOffset()` — computes vertical displacement at any x position
- `drawStrip()` — draws one vertical strip with correct color and wave offset
- `drawWavingFlag()` — loops through 80 strips to draw the full waving flag
- `drawMapleLeaf()` — draws the SVG leaf with wave deformation applied

**Challenge I faced:**

My first attempt shifted strips **horizontally** (left/right), which made the flag edges look jagged and unrealistic. The correct approach is to shift strips **vertically** (up/down), which is how a real fabric wave works. Switching the offset from x to y fixed the visual completely.

---

### 🔴 edom-jpg — Transformations, Keyboard Control, Timer & OpenGL Setup

**What I did:**

My responsibility was to wire everything together — set up the OpenGL window, apply the three required transformations to the maple leaf, implement keyboard controls, and drive the animation with a timer.

**How I did it:**

**3 Required Transformations:**

All three transformations are applied to the maple leaf inside a `glPushMatrix()` / `glPopMatrix()` block so they don't affect the flag background:

| # | Transformation | OpenGL Call | Effect |
|---|---|---|---|
| 1 | **Translation** | `glTranslatef(0.0f, 0.02f, 0.0f)` | Moves the leaf slightly upward to center it visually |
| 2 | **Rotation** | `glRotatef(0.0f, 0.0f, 0.0f, 1.0f)` | Rotates around the Z axis (value is 0° static, demonstrating the call) |
| 3 | **Scaling** | `glScalef(0.85f, 0.85f, 1.0f)` | Scales the leaf down to fit neatly inside the white section |

**Keyboard Controls:**

```
W / w  →  Toggle wave animation ON / OFF
ESC    →  Quit the program
```

**Timer (Animation Loop):**

The timer runs at approximately **60 FPS** using `glutTimerFunc(16, timer, 0)`. Each frame, if waving is active, `waveTime` is incremented by `0.04` to advance the wave forward.

**Reshape (Aspect Ratio):**

The `reshape()` function keeps the flag at its correct **2:1 aspect ratio** regardless of window size. It uses letterboxing/pillarboxing — computing a centered viewport that preserves proportions, then setting `glOrtho(-1, 1, -0.5, 0.5, -1, 1)`.

**OpenGL Setup:**

- `GLUT_DOUBLE` — double buffering to prevent flicker
- `GLUT_RGB` — standard RGB color mode
- `glClearColor(0.53, 0.81, 0.98)` — light blue sky background
- Window size: `1920 × 960` (matches the 2:1 flag ratio)

**Key functions I wrote:**
- `display()` — main render callback with all 3 transformations
- `keyboard()` — handles W/w wave toggle and ESC quit
- `timer()` — 60 FPS animation loop advancing `waveTime`
- `reshape()` — maintains 2:1 aspect ratio on window resize
- `main()` — OpenGL/GLUT initialization and event loop

**Challenge I faced:**

The `glPushMatrix()` / `glPopMatrix()` pair was critical. Without it, the transformations applied to the leaf would also affect the flag background, causing the entire scene to shift and scale. Wrapping only the leaf drawing in a matrix push/pop isolates the transformations correctly.

---

## 🎨 Color Implementation

All colors are based on the **official Canadian flag specification**:

| Element | Real Color | RGB Values | OpenGL Values |
|---|---|---|---|
| Red bars | Canadian Red (Pantone 032) | `(255, 0, 0)` | `(1.0, 0.0, 0.0)` |
| White center | Pure White | `(255, 255, 255)` | `(1.0, 1.0, 1.0)` |
| Maple leaf | Canadian Red | `(255, 0, 0)` | `(1.0, 0.0, 0.0)` |
| Background | Sky Blue | `(135, 206, 250)` | `(0.53, 0.81, 0.98)` |

---

## 🖥️ Primitives Used

| Primitive | Used For |
|---|---|
| `GL_QUADS` | Each vertical strip of the waving flag background |
| `GL_TRIANGLE_FAN` | Filled maple leaf shape |

---

## ✨ Bonus Feature — Wind Wave Animation

Our bonus feature is a **physically-inspired flag wave animation** triggered by pressing `W`.

**Why it is meaningful (not trivial):**
- Uses a **sine wave with a quadratic envelope** — not just a simple oscillation
- The left edge is **pinned** (zero displacement) simulating a flagpole
- The amplitude **grows toward the right** (free edge) — physically accurate
- The maple leaf **deforms with the flag** — it is not a separate static overlay
- The wave **travels** across the flag over time using `waveTime`

This goes well beyond a simple rotation or color change — it demonstrates understanding of parametric animation, envelope functions, and per-vertex displacement.

---

## ⚙️ How to Build and Run

```powershell
# Compile
g++ canada_flag.cpp -o canada_flag.exe -I./include -L./lib -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -std=c++11

# Run (freeglut.dll must be in the same folder)
.\canada_flag.exe
```

**Controls:**
| Key | Action |
|---|---|
| `W` | Toggle wave animation on/off |
| `ESC` | Quit |

---

## 🧩 Challenges We Faced

| Challenge | Who | Solution |
|---|---|---|
| SVG Y-axis is flipped vs OpenGL | FisihaM23 | Negate Y during normalization |
| Wave shifted horizontally (jagged edges) | meronkifle63-hub | Switch offset from X to Y axis |
| Transformations affecting the whole scene | edom-jpg | Wrap leaf drawing in `glPushMatrix()` / `glPopMatrix()` |
| File saved as UTF-16 (null bytes in compiler) | All | Re-save as UTF-8 before compiling |

---

## 📐 Coordinate System Summary

```
SVG Space          →   Normalized Model Space   →   OpenGL World Space
(large values,         (centered at origin,         (glOrtho projection)
 Y points down)         Y flipped up,                X: [-1, 1]
                        scaled to fit)               Y: [-0.5, 0.5]
```
