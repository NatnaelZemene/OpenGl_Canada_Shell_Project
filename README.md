# OpenGL Flag & Logo Rendering Project

**Course:** Computer Graphics
**Group:** 4 — Section 2
**Institution:** Gonder University

---

## Team Members

| Full Name | Student ID | GitHub Username | Project Role |
|---|---|---|---|
| Meron Kifle | GUR/01196/16 | meronkifle63-hub | Canada Flag — Rendering & Wave Animation |
| Natnael Zemene | GUR/02204/16 | NatnaelZemene | Shell Logo — SVG Parser & Geometry Builder |
| Hilina Shambel | GUR/01604/16 | hlinashambel | Shell Logo — GLU Tessellator & Primitives |
| Lidiya Assefa | GUR/02780/16 | liduassefa | Shell Logo — 3D Lighting, Transformations & Setup |
| Eden Shewaye | GUR/01134/16 | edom-jpg | Canada Flag — Transformations, Keyboard & Setup |
| Mulugeta Fissiha | GUR/01919/16 | FisihaM23 | Canada Flag — SVG Parser & Leaf Builder |

---

## Project Structure

```
OpenGl_Canada_Shell_Project/
|
|-- canada_flag.cpp          <- Canada Flag source code
|-- shell_logo.cpp           <- Shell Logo source code
|
|-- canda_flag_team.md       <- Canada Flag team detailed report
|-- shell_logo_team.md       <- Shell Logo team detailed report
|-- README.md                <- This file
|
|-- include/
|   +-- GL/
|       |-- freeglut.h
|       |-- freeglut_ext.h
|       +-- freeglut_std.h
|
+-- lib/
    +-- libfreeglut.a
```

Note: freeglut.dll is required at runtime but is not committed to the repository.
Download it separately — instructions below.

---

## How to Build and Run

### Prerequisites

You need the following installed:

| Tool | Purpose | Download |
|---|---|---|
| MinGW (g++) | C++ compiler | mingw.org |
| freeglut.dll | Runtime DLL | transmissionzero.co.uk/software/freeglut-devel |

Download freeglut for MinGW from the link above, extract it, and copy
freeglut\bin\freeglut.dll into the project root folder next to the .exe file.

---

### Option A — VS Code (Recommended)

1. Install the C/C++ extension by Microsoft in VS Code
2. Open the project folder in VS Code (File > Open Folder)
3. Open a terminal in VS Code (Terminal > New Terminal)
4. Run the compile command:

**Canada Flag:**
```
g++ canada_flag.cpp -o canada_flag.exe -I./include -L./lib -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -std=c++11
.\canada_flag.exe
```

**Shell Logo:**
```
g++ shell_logo.cpp -o shell_logo.exe -I./include -L./lib -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -std=c++11
.\shell_logo.exe
```

---

### Option B — Code::Blocks

1. Open Code::Blocks
2. Go to File > New > Project > Empty Project
3. Add the source file (canada_flag.cpp or shell_logo.cpp) to the project
4. Go to Project > Build Options > Linker Settings
5. Add these libraries under Link Libraries:
   - freeglut
   - opengl32
   - glu32
   - gdi32
   - winmm
6. Go to Search Directories > Compiler and add the include/ folder path
7. Go to Search Directories > Linker and add the lib/ folder path
8. Press F9 to Build and Run

---

### Option C — Command Line (PowerShell)

```
cd "path\to\OpenGl_Canada_Shell_Project"

g++ canada_flag.cpp -o canada_flag.exe -I./include -L./lib -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -std=c++11

g++ shell_logo.cpp -o shell_logo.exe -I./include -L./lib -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -std=c++11

.\canada_flag.exe
.\shell_logo.exe
```

---

## Controls

### Canada Flag
| Key | Action |
|---|---|
| `W` | Toggle wind wave animation on/off |
| `R` | Toggle maple leaf auto-spin (rotation) |
| `+` / `=` | Scale the maple leaf up |
| `-` | Scale the maple leaf down |
| `←` `→` `↑` `↓` | Translate (move) the entire flag |
| `ESC` | Quit |

### Shell Logo
| Key | Action |
|---|---|
| R | Start rotation and breathing animation |
| S | Stop animation |

---

## Why We Used Complex Features Instead of Basic GLUT Primitives

This section addresses the question: "Why didn't you just use basic glBegin(GL_POLYGON)
or hardcoded coordinates?"

### The Problem with Basic Primitives

The assignment gave us real-world logos and flags to render — not simple triangles or
rectangles. Both the Canadian maple leaf and the Shell logo have:

- Smooth curves (not straight lines)
- Complex concave shapes (inward curves, holes, multiple contours)
- Precise proportions that must match the real design

If we had used basic hardcoded coordinates with GL_POLYGON, we would have faced
these problems:

---

### Canada Flag — Why We Used SVG Parsing

The maple leaf has 11 points and smooth curved edges. Manually guessing (x, y)
coordinates for each point would produce a rough, inaccurate shape that does not
look like the real Canadian maple leaf.

Instead, we used the official Canadian flag SVG path data — the same vector data
used in the actual flag specification. This gave us:

- Mathematically precise leaf geometry
- Smooth Bezier curves instead of jagged straight lines
- Correct proportions guaranteed by the official source

**Why not just use GL_POLYGON with guessed points?**

GL_POLYGON with manually guessed coordinates would produce a rough approximation.
The assignment requires correct real-world colors — by the same logic, we applied
correct real-world geometry. Using the official SVG data is the most accurate way
to achieve this. We still use GL_TRIANGLE_FAN and GL_QUADS as required primitives —
the SVG parser only provides the vertex data, not a different rendering method.

---

### Shell Logo — Why We Used GLU Tessellator

The Shell logo is a concave polygon with an inner cutout. OpenGL's basic GL_POLYGON
primitive only works correctly for convex shapes. The Shell logo is concave — it has
inward curves and the inner detail creates a hole.

If we had used GL_POLYGON directly:
- The shape would render incorrectly (triangles crossing the wrong way)
- The inner cutout would be filled instead of transparent
- The result would not look like the Shell logo at all

The GLU Tessellator is the standard OpenGL solution for this exact problem. It:
- Breaks the concave polygon into triangles automatically
- Handles multiple contours (outer boundary + inner cutout)
- Uses a winding rule (GLU_TESS_WINDING_ODD) to correctly determine filled vs hole areas

This is not an advanced feature we chose for complexity — it is the correct and
necessary tool for rendering a concave polygon in OpenGL. The tessellator still
uses GL_POLYGON, GL_LINE_LOOP, and GL_POINTS as the final output primitives.

---

### Summary Table

| Feature | Why We Used It | Basic Alternative and Why It Fails |
|---|---|---|
| SVG path parser | Official geometry for accurate maple leaf | Hardcoded guesses produce wrong shape |
| Cubic Bezier sampling | Smooth curves on leaf and shell outline | Straight lines produce jagged unrealistic edges |
| GLU Tessellator | Only correct way to fill a concave polygon | GL_POLYGON breaks for concave shapes |
| 3D perspective + lighting | Demonstrates 3D concepts from the course | Flat 2D misses depth and lighting requirements |
| Wave animation | Meaningful bonus with physics-based math | Simple rotation is trivial and not meaningful |

We did not use these features to show off. We used them because the shapes we were
assigned require them to be rendered correctly. The course requirement is to render
the assigned flag/logo accurately, and accuracy demanded these tools.

---

## Assignment Requirements Checklist

| Requirement | Canada Flag | Shell Logo |
|---|---|---|
| Basic primitives | GL_QUADS, GL_TRIANGLE_FAN | GL_POLYGON (tessellated), GL_LINE_LOOP, GL_POINTS |
| Color handling (RGB) | Canadian Red (1.0, 0.0, 0.0), White | Shell Yellow (0.984, 0.807, 0.066), Shell Red (0.866, 0.113, 0.129) |
| At least 3 transformations | Translate (arrow keys), Rotate (R key), Scale (+/- keys) on leaf | Translate, Scale (breathing), Rotate (Y-axis + X-tilt) |
| Correct real-world colors | Official Canadian flag specification | Official Shell brand color guidelines |
| Working OpenGL program | Yes | Yes |
| Bonus feature | Wind wave animation (W key) + interactive transforms (arrow keys, R, +/-) | Orbiting light + 3D animation (R key) |

---

## Detailed Reports

For a full explanation of each team member's contribution, implementation details,
and challenges faced:

- Canada Flag Team Report: canda_flag_team.md
- Shell Logo Team Report:  shell_logo_team.md