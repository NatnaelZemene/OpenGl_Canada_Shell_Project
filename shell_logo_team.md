# 🐚 Shell Logo — OpenGL Project Report

**Course:** Computer Graphics  
**Project:** Flag & Logo Rendering with Transformations  
**Team Members:** NatnaelZemene · hlinashambel · liduassefa  
**Language:** C++ with OpenGL (FreeGLUT + GLU + Legacy Immediate Mode)

---

## 📌 Project Overview

Our team was assigned the **Shell (oil company) logo** — the iconic red and yellow scallop shell. The logo was provided as a black silhouette SVG. Our goal was to render it with the correct real-world brand colors, apply the required transformations, set up 3D lighting for a glossy effect, and add a meaningful bonus animation feature.

We divided the work into three equal parts: geometry parsing and normalization, rendering with the GLU tessellator, and the full 3D OpenGL setup with transformations and animation.

---

## 👥 Team Contributions

### 🔵 NatnaelZemene — SVG Path Parser & Shell Geometry Builder

**What I did:**

My responsibility was to extract the shell shape from its SVG path data and convert it into OpenGL-ready geometry. The shell logo is a complex concave polygon with smooth curves — it cannot be drawn with simple rectangles or circles. I had to parse the official SVG path and produce accurate vertex data.

**How I did it:**

I wrote a complete SVG path parser that handles all the commands present in the shell's path:

| SVG Command | Meaning |
|---|---|
| `M / m` | Move to (absolute / relative) |
| `L / l` | Line to |
| `H / h` | Horizontal line |
| `V / v` | Vertical line |
| `C / c` | Cubic Bezier curve |
| `Z / z` | Close path |

The shell outline is almost entirely made of **cubic Bezier curves** (`C/c` commands). I implemented the standard cubic Bezier interpolation formula:

```
B(t) = (1-t)³·P0 + 3(1-t)²t·P1 + 3(1-t)t²·P2 + t³·P3
```

I used **40 samples per curve segment** (`kCurveSamples = 40`) to produce a very smooth, high-resolution outline — much higher than the flag's 14 samples, because the shell has tighter curves that need more detail.

The SVG path produces **two subpaths** — the outer shell boundary and the inner cutout. Both are needed for the tessellator to correctly fill the shape with the hole.

After parsing, I normalized all subpaths into OpenGL coordinates:
- Computed the bounding box across all subpaths
- Centered everything around the origin `(0, 0)`
- Scaled uniformly to fit within `kFit = 1.6` units
- **Flipped Y** because SVG Y grows downward, OpenGL Y grows upward

The result is stored in `gShellSubpaths` — a list of subpaths, each containing normalized `(x, y)` points.

**Key functions I wrote:**
- `cubic()` — evaluates a cubic Bezier curve at parameter t
- `skipSeparators()` — skips whitespace and commas while parsing
- `readFloat()` — reads one float from the SVG path string
- `parseSvgPath()` — full parser returning a list of subpaths
- `initShell()` — parses the SVG, normalizes all subpaths, stores in `gShellSubpaths`

**Challenge I faced:**

The shell SVG has two separate closed subpaths (outer boundary + inner detail). My first version only kept the largest subpath, which lost the inner shape. I fixed this by keeping **all subpaths** and passing them all to the tessellator — which then uses the winding rule to correctly determine what is inside and outside.

---

### 🟢 hlinashambel — Shell Rendering with GLU Tessellator & 3 Primitives

**What I did:**

My responsibility was to take the normalized subpaths from NatnaelZemene and render the shell on screen using all three required OpenGL primitives: filled polygon, outline, and points.

**How I did it:**

**The Core Problem — Concave Polygon:**

The shell is a **concave polygon** (it has inward curves and holes). OpenGL's basic `glBegin(GL_POLYGON)` only works correctly for convex shapes. For concave shapes, you must use the **GLU Tessellator** — a library that breaks the complex polygon into triangles that OpenGL can render.

**GLU Tessellator Setup:**

```cpp
GLUtesselator* tess = gluNewTess();
gluTessCallback(tess, GLU_TESS_BEGIN,  &tessBegin);   // calls glBegin()
gluTessCallback(tess, GLU_TESS_VERTEX, &tessVertex);  // calls glVertex3dv()
gluTessCallback(tess, GLU_TESS_END,    &tessEnd);     // calls glEnd()
gluTessCallback(tess, GLU_TESS_ERROR,  &tessError);   // error reporting
gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
```

The `GLU_TESS_WINDING_ODD` rule means: areas enclosed by an odd number of contours are filled, even number are holes. This correctly handles the shell's inner cutout.

I submit all subpaths as separate contours within one polygon:

```cpp
gluTessBeginPolygon(tess, nullptr);
for each subpath:
    gluTessBeginContour(tess);
    for each vertex: gluTessVertex(...)
    gluTessEndContour(tess);
gluTessEndPolygon(tess);
```

**Important:** I pre-allocated all vertex data in a `vector<vector<GLdouble>>` with `reserve()` before starting tessellation. This prevents pointer invalidation — the tessellator stores raw pointers to vertex data, so the data must not move in memory during tessellation.

**3 Primitives:**

| # | Primitive | Color | Purpose |
|---|---|---|---|
| 1 | `GL_POLYGON` (via tessellator) | Shell Yellow | Filled shell shape |
| 2 | `GL_LINE_LOOP` | Shell Red | Crisp outline around each subpath |
| 3 | `GL_POINTS` | Shell Red | Highlights individual vertices |

Lighting is enabled for the filled polygon (to show the 3D glossy effect from liduassefa's lighting setup), then disabled for the outlines and points so they stay a pure, crisp color.

**Key functions I wrote:**
- `tessBegin()`, `tessEnd()`, `tessVertex()`, `tessError()` — GLU tessellator callbacks
- `drawShell()` — full rendering function using all 3 primitives

**Challenge I faced:**

The tessellator crashed with a memory error on my first attempt. The problem was that I was passing pointers to local `GLdouble` arrays that went out of scope. The fix was to store all vertex data in a `std::vector<std::vector<GLdouble>>` that lives for the entire duration of the tessellation call, and use `reserve()` to prevent reallocation.

---

### 🔴 liduassefa — 3D Lighting, Transformations, Animation & OpenGL Setup

**What I did:**

My responsibility was to set up the full 3D OpenGL environment — perspective projection, lighting, material properties, the three required transformations, keyboard controls, and the bonus animation feature.

**How I did it:**

**3D Perspective Projection:**

Unlike the flag which uses flat 2D orthographic projection, the shell uses **3D perspective**:

```cpp
gluPerspective(45.0, 800.0/600.0, 0.1, 100.0);
// (Field of View, Aspect Ratio, Near plane, Far plane)
```

The camera is placed at `(0, 0, 3)` looking at the origin using `gluLookAt()`.

**3D Lighting Setup:**

I set up a single positional light (`GL_LIGHT0`) with three components:

| Component | Values | Effect |
|---|---|---|
| Ambient | `(0.3, 0.3, 0.3)` | Base illumination in shadow areas |
| Diffuse | `(0.7, 0.7, 0.7)` | Main directional light |
| Specular | `(1.0, 1.0, 1.0)` | Bright white glossy highlight |

Material shininess is set to `50.0` — a high value that produces a small, sharp specular highlight, giving the shell a glossy plastic appearance.

`GL_COLOR_MATERIAL` is enabled so `glColor3f()` still controls the base color while lighting is active.

**3 Required Transformations:**

All three are applied inside a `glPushMatrix()` / `glPopMatrix()` block:

| # | Transformation | OpenGL Call | Effect |
|---|---|---|---|
| 1 | **Translation** | `glTranslatef(0, 0, 0)` | Centers the shell at the origin |
| 2 | **Scale** | `glScalef(s, s, s)` | Breathing effect: `s = 1 + 0.1·sin(gScaleTime)` |
| 3 | **Rotation** | `glRotatef(gAngle, 0, 1, 0)` + `glRotatef(15, 1, 0, 0)` | Y-axis spin + permanent 15° X tilt |

**Bonus Feature — Orbiting Light + Smooth Animation:**

The light source **orbits the shell** in sync with the rotation:

```cpp
GLfloat lightPos[] = {
    2.0f * cos(gAngle * π / 180),   // x
    2.0f * sin(gAngle * π / 180),   // y
    2.0f, 1.0f                       // z, w=1 (positional)
};
glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
```

As the shell spins, the light moves with it, creating dynamic specular highlights that sweep across the surface — demonstrating the 3D lighting model in action.

**Keyboard Controls:**

```
R / r  →  Start rotation + breathing animation
S / s  →  Stop animation (freeze in place)
```

**Timer:**

Runs at **~60 FPS** using `glutTimerFunc(16, timer, 0)`. Each frame:
- `gAngle += 0.5°` (Y-axis spin)
- `gScaleTime += 0.05` (breathing scale)

**Key functions I wrote:**
- `keyboard()` — R starts animation, S stops it
- `display()` — 3D camera, orbiting light, 3 transformations, drawShell call
- `timer()` — 60 FPS loop updating gAngle and gScaleTime
- `init()` — full OpenGL setup: depth test, lighting, materials, perspective projection
- `main()` — GLUT initialization, window creation, callback registration

**Challenge I faced:**

The shell appeared completely black when lighting was first enabled. The problem was that `glColor3f()` is ignored when lighting is on unless `GL_COLOR_MATERIAL` is enabled. Adding `glEnable(GL_COLOR_MATERIAL)` and `glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE)` fixed it — now `glColor3f()` sets the material's ambient and diffuse color, and lighting works correctly on top of it.

---

## 🎨 Color Implementation

All colors are based on the **official Shell brand color guidelines**:

| Element | Brand Color | RGB Values | OpenGL Values |
|---|---|---|---|
| Shell fill | Shell Yellow (Pantone 116) | `(251, 206, 17)` | `(0.984, 0.807, 0.066)` |
| Shell outline | Shell Red (Pantone 485) | `(221, 29, 33)` | `(0.866, 0.113, 0.129)` |
| Background | White | `(255, 255, 255)` | `(1.0, 1.0, 1.0)` |

---

## 🖥️ Primitives Used

| Primitive | Used For |
|---|---|
| `GL_POLYGON` (via GLU tessellator) | Filled shell shape (handles concave polygon correctly) |
| `GL_LINE_LOOP` | Red outline around each subpath contour |
| `GL_POINTS` | Red dots highlighting individual vertices |

---

## ✨ Bonus Feature — 3D Orbiting Light Animation

Our bonus feature is a **3D animated shell with an orbiting light source**, triggered by pressing `R`.

**Why it is meaningful (not trivial):**
- Uses **3D perspective projection** instead of flat 2D orthographic
- A **positional light orbits** the shell in sync with the rotation — not a fixed light
- The **specular highlight sweeps** across the surface as the shell spins, demonstrating the Phong lighting model
- A **breathing scale** (`sin`-based) adds a subtle pulsing effect
- The shell has a **permanent 15° X-axis tilt** so the 3D depth is always visible

This goes well beyond a simple rotation — it demonstrates 3D camera setup, the full OpenGL lighting pipeline, material properties, and synchronized multi-transformation animation.

---

## ⚙️ How to Build and Run

```powershell
# Compile
g++ shell_logo.cpp -o shell_logo.exe -I./include -L./lib -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -std=c++11

# Run (freeglut.dll must be in the same folder)
.\shell_logo.exe
```

**Controls:**
| Key | Action |
|---|---|
| `R` | Start rotation + breathing animation |
| `S` | Stop animation |

---

## 🧩 Challenges We Faced

| Challenge | Who | Solution |
|---|---|---|
| Shell has two subpaths (outer + inner) | NatnaelZemene | Keep all subpaths, pass all to tessellator |
| SVG Y-axis flipped vs OpenGL | NatnaelZemene | Negate Y during normalization |
| Tessellator crash (dangling pointer) | hlinashambel | Pre-allocate vertex data with `reserve()` |
| Shell appeared black with lighting on | liduassefa | Enable `GL_COLOR_MATERIAL` |
| File saved as UTF-16 (null bytes) | All | Re-save as UTF-8 before compiling |

---

## 📐 Architecture Overview

```
parseSvgPath()          →   gShellSubpaths (normalized)
        ↓
initShell()             →   bounding box → center → scale → flip Y
        ↓
drawShell()             →   GLU Tessellator (fill) + LINE_LOOP + POINTS
        ↓
display()               →   gluLookAt + glTranslate + glScale + glRotate
        ↓
timer()                 →   gAngle++, gScaleTime++ @ 60 FPS
```

---

## 📊 Coordinate System Summary

```
SVG Space              →   Normalized Model Space   →   OpenGL 3D Space
(large pixel values,       (centered at origin,         (perspective projection)
 Y points down,             Y flipped up,                Camera at (0,0,3)
 two subpaths)              scaled to kFit=1.6)          looking at origin
```
