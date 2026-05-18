// Shell Logo - OpenGL
// Commit 1: SVG path parser and shell geometry builder — NatnaelZemene

#include <GL/freeglut.h>
#include <GL/glu.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#ifndef CALLBACK
#define CALLBACK
#endif

// ─── GLU Tessellator callbacks (needed by drawShell) ─────────────────────────
void CALLBACK tessBegin(GLenum type) { glBegin(type); }
void CALLBACK tessEnd()              { glEnd(); }
void CALLBACK tessVertex(void* data) {
    const GLdouble* ptr = (const GLdouble*)data;
    glVertex3dv(ptr);
}
void CALLBACK tessError(GLenum errorCode) {
    const GLubyte* err = gluErrorString(errorCode);
    if (err) std::cerr << "Tessellation Error: " << err << std::endl;
}

// ─── Data structures ──────────────────────────────────────────────────────────
struct Point { float x; float y; };

static std::vector<std::vector<Point>> gShellSubpaths;
static constexpr int   kCurveSamples = 40;  // high resolution for smooth curves
static constexpr float kFit          = 1.6f;

// ─── Cubic Bezier interpolation ───────────────────────────────────────────────
float cubic(float p0, float p1, float p2, float p3, float t) {
    const float it = 1.0f - t;
    return it*it*it*p0 + 3.0f*it*it*t*p1 + 3.0f*it*t*t*p2 + t*t*t*p3;
}

// ─── SVG parser helpers ───────────────────────────────────────────────────────
void skipSeparators(const std::string& s, size_t& i) {
    while (i < s.size()) {
        const char c = s[i];
        if (std::isspace(static_cast<unsigned char>(c)) || c == ',') { ++i; continue; }
        break;
    }
}

bool readFloat(const std::string& s, size_t& i, float& out) {
    skipSeparators(s, i);
    if (i >= s.size()) return false;
    char* endPtr = nullptr;
    const char* start = s.c_str() + i;
    const double v = std::strtod(start, &endPtr);
    if (endPtr == start) return false;
    i = static_cast<size_t>(endPtr - s.c_str());
    out = static_cast<float>(v);
    return true;
}

// ─── Full SVG path parser (M/m L/l H/h V/v C/c Z/z) ─────────────────────────
std::vector<std::vector<Point>> parseSvgPath(const std::string& d) {
    std::vector<std::vector<Point>> subpaths;
    std::vector<Point> current;
    float cx=0,cy=0,sx=0,sy=0; char cmd=0; size_t i=0;

    while (i < d.size()) {
        skipSeparators(d,i); if (i>=d.size()) break;
        if (std::isalpha(static_cast<unsigned char>(d[i]))) { cmd=d[i++]; }
        else if (cmd==0) break;

        switch (cmd) {
            case 'M': case 'm': {
                float x=0,y=0; if (!readFloat(d,i,x)||!readFloat(d,i,y)) break;
                if (!current.empty()) { subpaths.push_back(current); current.clear(); }
                if (cmd=='m'){cx+=x;cy+=y;}else{cx=x;cy=y;} sx=cx;sy=cy;
                current.push_back({cx,cy});
                while(true){size_t p=i;float lx=0,ly=0;if(!readFloat(d,p,lx)||!readFloat(d,p,ly))break;i=p;if(cmd=='m'){cx+=lx;cy+=ly;}else{cx=lx;cy=ly;}current.push_back({cx,cy});}
                break;
            }
            case 'L': case 'l': {
                while(true){size_t p=i;float x=0,y=0;if(!readFloat(d,p,x)||!readFloat(d,p,y))break;i=p;if(cmd=='l'){cx+=x;cy+=y;}else{cx=x;cy=y;}current.push_back({cx,cy});}
                break;
            }
            case 'H': case 'h': {
                while(true){size_t p=i;float x=0;if(!readFloat(d,p,x))break;i=p;if(cmd=='h')cx+=x;else cx=x;current.push_back({cx,cy});}
                break;
            }
            case 'V': case 'v': {
                while(true){size_t p=i;float y=0;if(!readFloat(d,p,y))break;i=p;if(cmd=='v')cy+=y;else cy=y;current.push_back({cx,cy});}
                break;
            }
            case 'C': case 'c': {
                while(true){
                    size_t p=i; float x1=0,y1=0,x2=0,y2=0,x=0,y=0;
                    if(!readFloat(d,p,x1)||!readFloat(d,p,y1)||!readFloat(d,p,x2)||!readFloat(d,p,y2)||!readFloat(d,p,x)||!readFloat(d,p,y)) break;
                    i=p;
                    float p0x=cx,p0y=cy;
                    float p1x=(cmd=='c')?(cx+x1):x1, p1y=(cmd=='c')?(cy+y1):y1;
                    float p2x=(cmd=='c')?(cx+x2):x2, p2y=(cmd=='c')?(cy+y2):y2;
                    float p3x=(cmd=='c')?(cx+x) :x,  p3y=(cmd=='c')?(cy+y) :y;
                    for(int step=1;step<=kCurveSamples;++step){
                        float t=static_cast<float>(step)/kCurveSamples;
                        current.push_back({cubic(p0x,p1x,p2x,p3x,t),cubic(p0y,p1y,p2y,p3y,t)});
                    }
                    cx=p3x; cy=p3y;
                }
                break;
            }
            case 'Z': case 'z': {
                if(!current.empty()){current.push_back({sx,sy});subpaths.push_back(current);current.clear();}
                cx=sx; cy=sy;
                break;
            }
            default: ++i; break;
        }
    }
    if (!current.empty()) subpaths.push_back(current);
    return subpaths;
}

// ─── Parse Shell SVG and normalize to OpenGL coordinates ─────────────────────
void initShell() {
    static const std::string kSvgPath = "M 246.06805,479.40149 C 226.21232,463.8826 198.81128,472.66654 174.96028,470.1267 150.2468,470.01693 125.53325,469.9232 100.81974,469.82013 96.681875,438.16588 94.334252,406.03397 88.6945,374.76437 64.112336,353.54879 35.228923,337.61391 10.988895,315.96935 -9.2916422,226.15896 26.863666,128.24492 97.619925,70.166774 167.53761,9.7904766 271.63478,-7.1364097 356.68823,29.486475 456.93608,69.911655 525.65107,178.6574 514.3987,286.88895 c 2.47114,23.57878 -11.79702,40.07735 -31.69233,50.3805 -16.22423,14.33823 -40.33721,23.7702 -51.7047,41.15712 -3.4734,30.15432 -7.20465,60.27898 -10.56602,90.44605 -43.50077,1.78381 -87.09531,0.68986 -130.6388,0.98877 -11.72267,10.67983 -28.55914,15.60768 -43.7288,9.5401 z m 23.86759,-41.90918 c 9.76479,-5.53542 17.55532,-14.72287 29.76963,-11.63092 27.21325,0 54.4265,0 81.63975,0 4.14033,-25.56587 6.02833,-51.48022 10.73711,-76.95395 25.06025,-18.4656 50.78223,-36.04212 75.4482,-55.03596 7.57119,-23.58583 6.01634,-51.09614 -5.18738,-73.45426 -42.82456,42.87637 -84.79997,86.62204 -128.16209,128.94417 40.97136,-51.21851 83.98581,-100.85801 123.54428,-153.17162 -2.33634,-20.87069 -16.55575,-40.91352 -31.39654,-55.81368 -13.57794,16.47776 -23.75668,40.28143 -36.07549,59.41719 -26.5222,45.62877 -52.60923,91.52404 -79.26215,137.06811 32.69067,-72.19281 68.17044,-143.14347 99.58718,-215.89677 -9.34656,-18.34253 -31.25363,-32.193862 -51.44279,-38.004466 -10.02612,8.92926 -9.74314,33.426946 -16.32994,47.454476 -18.3699,63.80867 -36.20465,127.78714 -54.72725,191.54218 C 303.60352,238.35227 321.25671,155.15202 337.43565,71.67057 318.32811,59.633148 294.146,54.951877 271.93687,57.859388 267.6137,145.9884 263.78605,234.16315 261.16374,322.36139 255.59675,234.18022 254.7921,145.71333 248.99456,57.551674 226.61622,55.959795 201.93445,58.488986 183.53502,72.361386 200.23836,155.52932 216.85568,238.71464 233.66411,321.86139 209.74699,242.8214 187.79736,162.84094 164.8349,83.361386 142.34689,85.253865 124.29049,102.99435 109.89252,118.9973 140.9219,192.51458 177.4174,263.85125 210.05675,336.59449 171.90031,271.46077 134.63181,205.66738 96.430445,140.50474 79.928855,153.2417 65.994099,175.71029 63.462171,196.26085 103.79487,249.97582 148.56464,300.32424 189.79366,353.36139 146.14803,309.21125 102.64715,264.91754 59.071465,220.69807 c -12.007701,22.15153 -12.521918,49.34308 -6.13582,73.30627 25.649721,18.49312 51.299445,36.98624 76.949165,55.47936 3.16897,25.04153 6.44485,50.06991 9.48311,75.12769 32.1279,3.54022 65.17322,-1.26998 97.15499,2.85515 9.11313,7.69478 21.05486,16.73132 33.41273,10.02577 z";

    const std::vector<std::vector<Point>> rawSubpaths = parseSvgPath(kSvgPath);
    if (rawSubpaths.empty()) return;

    float minX=rawSubpaths[0][0].x, maxX=rawSubpaths[0][0].x;
    float minY=rawSubpaths[0][0].y, maxY=rawSubpaths[0][0].y;
    for (const auto& sp : rawSubpaths)
        for (const Point& p : sp) {
            minX=std::min(minX,p.x); maxX=std::max(maxX,p.x);
            minY=std::min(minY,p.y); maxY=std::max(maxY,p.y);
        }

    const float srcW  = std::max(1e-6f, maxX-minX);
    const float srcH  = std::max(1e-6f, maxY-minY);
    const float srcCX = 0.5f*(minX+maxX);
    const float srcCY = 0.5f*(minY+maxY);
    const float scale = std::min(kFit/srcW, kFit/srcH);

    gShellSubpaths.clear();
    for (const auto& sp : rawSubpaths) {
        std::vector<Point> norm;
        norm.reserve(sp.size());
        for (const Point& p : sp)
            norm.push_back({ (p.x-srcCX)*scale, -(p.y-srcCY)*scale });
        gShellSubpaths.push_back(norm);
    }
}

// ─── Stub main (replaced in Commit 3) ────────────────────────────────────────
void display() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glutSwapBuffers(); }
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Shell Logo");
    initShell();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
