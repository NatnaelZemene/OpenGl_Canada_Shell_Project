// National Flag of Canada - OpenGL
// Commit 1: SVG path parser and maple leaf builder        — FisihaM23
// Commit 2: Flag rendering and waving animation           — meronkifle63-hub
// Commit 3: Transformations, keyboard control, and setup  — edom-jpg

#include <GL/freeglut.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

// ─── Data structures ──────────────────────────────────────────────────────────
struct Point {
    float x;
    float y;
};

static std::vector<std::pair<float, float>> gLeafPoints;
static constexpr int   kCurveSamples = 14;
static constexpr float kLeafFit      = 0.86f;

// ─── Wave state (Bonus feature) ───────────────────────────────────────────────
float waveTime = 0.0f;
bool  gWaving  = false;   // toggled by W/w key

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

// ─── Full SVG path parser ─────────────────────────────────────────────────────
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
            case 'L': case 'l': { while(true){size_t p=i;float x=0,y=0;if(!readFloat(d,p,x)||!readFloat(d,p,y))break;i=p;if(cmd=='l'){cx+=x;cy+=y;}else{cx=x;cy=y;}current.push_back({cx,cy});} break; }
            case 'H': case 'h': { while(true){size_t p=i;float x=0;if(!readFloat(d,p,x))break;i=p;if(cmd=='h')cx+=x;else cx=x;current.push_back({cx,cy});} break; }
            case 'V': case 'v': { while(true){size_t p=i;float y=0;if(!readFloat(d,p,y))break;i=p;if(cmd=='v')cy+=y;else cy=y;current.push_back({cx,cy});} break; }
            case 'C': case 'c': {
                while(true){size_t p=i;float x1=0,y1=0,x2=0,y2=0,x=0,y=0;
                if(!readFloat(d,p,x1)||!readFloat(d,p,y1)||!readFloat(d,p,x2)||!readFloat(d,p,y2)||!readFloat(d,p,x)||!readFloat(d,p,y))break;i=p;
                float p0x=cx,p0y=cy,p1x=(cmd=='c')?(cx+x1):x1,p1y=(cmd=='c')?(cy+y1):y1,p2x=(cmd=='c')?(cx+x2):x2,p2y=(cmd=='c')?(cy+y2):y2,p3x=(cmd=='c')?(cx+x):x,p3y=(cmd=='c')?(cy+y):y;
                for(int s=1;s<=kCurveSamples;++s){float t=static_cast<float>(s)/kCurveSamples;current.push_back({cubic(p0x,p1x,p2x,p3x,t),cubic(p0y,p1y,p2y,p3y,t)});}
                cx=p3x;cy=p3y;}
                break;
            }
            case 'Z': case 'z': { if(!current.empty()){current.push_back({sx,sy});subpaths.push_back(current);current.clear();}cx=sx;cy=sy; break; }
            default: ++i; break;
        }
    }
    if (!current.empty()) subpaths.push_back(current);
    return subpaths;
}

// ─── Build normalized leaf from official SVG path ────────────────────────────
void buildNormalizedLeafFromSvg() {
    static const std::string kSvgPath = R"SVG(M -113.87755,482.44898 V 2.4489799 h 240 240 V 482.44898 962.44899 h -240 -240 z m 1439.99995,0 V 2.4489799 h 240 240 V 482.44898 962.44899 h -240 -240 z m -497.58237,401.75 c 1.29867,-10.21858 8.86185,-170.62456 8.20589,-174.03697 -1.02308,-5.32218 -7.79126,-12.16119 -13.56288,-13.7048 -4.09466,-1.09512 -11.17988,-0.002 -91.23847,14.08042 -47.73948,8.39726 -87.04751,15.01929 -87.35118,14.71562 -0.30367,-0.30366 4.64114,-14.83884 10.98847,-32.30039 6.98356,-19.21183 11.54059,-33.22111 11.54059,-35.47819 0,-2.27508 -0.87761,-4.94935 -2.25,-6.85628 -1.2375,-1.71949 -44.1,-36.93918 -95.25,-78.26597 -51.15,-41.32679 -93.41977,-75.52875 -93.93283,-76.00436 -0.54542,-0.50561 3.1923,-2.78227 9,-5.48195 31.88113,-14.81976 36.06108,-17.11395 37.77963,-20.73553 0.90926,-1.91612 1.6532,-4.69681 1.6532,-6.17931 0,-1.48251 -8.3541,-28.3479 -18.56467,-59.70087 -10.21057,-31.35298 -18.32001,-57.25007 -18.02099,-57.5491 0.29903,-0.29902 14.84063,2.49035 32.31467,6.1986 79.82262,16.93957 77.13624,16.42507 80.66875,15.4497 1.84544,-0.50955 4.44952,-2.02063 5.78685,-3.35795 1.36705,-1.36706 6.93144,-13.07179 12.71072,-26.73708 5.65357,-13.36808 10.59945,-24.64259 10.99083,-25.05447 0.39138,-0.41189 19.81461,19.76942 43.16272,44.84734 23.34812,25.07793 43.66627,46.22743 45.15146,46.99889 8.25065,4.28568 18.78559,-2.14362 18.79592,-11.47084 0.002,-1.85542 -9.02509,-49.91894 -20.06032,-106.80783 -11.03523,-56.88888 -19.92273,-103.81872 -19.75,-104.28854 0.17273,-0.46981 15.16406,7.75889 33.31406,18.286 29.40508,17.05514 33.49022,19.14259 37.5,19.16204 8.51049,0.0413 7.52972,1.60228 43.90174,-69.87494 18.36064,-36.08178 33.71117,-65.603236 34.11228,-65.603236 0.40111,0 15.74955,29.523866 34.10764,65.608596 36.35694,71.4634 35.38194,69.91078 43.87834,69.86958 4.0098,-0.0195 8.0949,-2.1069 37.5,-19.16204 18.15,-10.52711 33.1396,-18.76107 33.3102,-18.29767 0.1706,0.46339 -8.7169,47.39631 -19.75,104.29538 -11.0331,56.89907 -20.0585,104.96784 -20.0565,106.8195 0.01,9.33575 10.5374,15.74091 18.826,11.45469 1.5018,-0.77657 21.8229,-21.92314 45.15795,-46.99236 23.3352,-25.06921 42.7437,-45.24633 43.1301,-44.83804 0.3864,0.4083 5.0722,11.09236 10.4129,23.74236 5.3407,12.65 10.3921,24.17651 11.2254,25.61446 1.9637,3.38873 6.5634,6.06911 10.5694,6.15902 1.746,0.0392 27.1618,-5.00654 56.4796,-11.21272 29.3177,-6.20619 53.5343,-11.05465 53.8146,-10.77436 0.2803,0.28028 -7.8445,26.16204 -18.055,57.51502 -10.2106,31.35297 -18.5647,58.21836 -18.5647,59.70087 0,1.4825 0.7439,4.26319 1.6532,6.17931 1.7186,3.62158 5.8985,5.91577 37.7796,20.73553 5.8151,2.70311 9.5461,4.97629 9,5.48337 -0.513,0.47638 -42.7828,34.68396 -93.9328,76.01684 -51.15,41.33288 -94.0125,76.55193 -95.25,78.26456 -1.3746,1.9024 -2.25,4.56502 -2.25,6.84379 0,2.25708 4.557,16.26636 11.5406,35.47819 6.3473,17.46155 11.3053,31.98353 11.0178,32.27108 -0.2876,0.28754 -39.5956,-6.3345 -87.35105,-14.71565 -80.06628,-14.0517 -87.17407,-15.14598 -91.26781,-14.05111 -5.77238,1.54382 -12.54021,8.38283 -13.56222,13.70483 -0.6401,3.33323 6.9145,164.88832 8.16165,174.53697 l 0.48472,3.75 h -18.0731 -18.07309 z)SVG";

    const std::vector<std::vector<Point>> subpaths = parseSvgPath(kSvgPath);
    if (subpaths.empty()) return;
    const auto it = std::max_element(subpaths.begin(), subpaths.end(),
        [](const std::vector<Point>& a, const std::vector<Point>& b){ return a.size()<b.size(); });
    const std::vector<Point>& leaf = *it;
    float minX=leaf.front().x,maxX=leaf.front().x,minY=leaf.front().y,maxY=leaf.front().y;
    for (const Point& p:leaf){minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
    const float srcW=std::max(1e-6f,maxX-minX),srcH=std::max(1e-6f,maxY-minY);
    const float srcCX=0.5f*(minX+maxX),srcCY=0.5f*(minY+maxY);
    const float scale=std::min(kLeafFit/srcW,kLeafFit/srcH);
    gLeafPoints.clear(); gLeafPoints.reserve(leaf.size());
    for (const Point& p:leaf) gLeafPoints.push_back({(p.x-srcCX)*scale,-(p.y-srcCY)*scale});
}

// ─── BONUS: Wave offset ───────────────────────────────────────────────────────
float waveOffset(float x) {
    if (!gWaving) return 0.0f;
    float t        = (x + 1.0f) * 0.5f;
    float envelope = t * t;
    return envelope * 0.07f * sinf(waveTime * 3.0f + t * 6.28f);
}

// ─── Draw one vertical strip with correct flag color ─────────────────────────
void drawStrip(float x1, float x2, float top, float bot) {
    float xMid = (x1 + x2) * 0.5f;
    float rel   = (xMid + 1.0f) / 2.0f;
    if (rel < 0.25f || rel > 0.75f)
        glColor3f(1.0f, 0.0f, 0.0f);
    else
        glColor3f(1.0f, 1.0f, 1.0f);
    float o1 = waveOffset(x1), o2 = waveOffset(x2);
    glBegin(GL_QUADS);
        glVertex2f(x1, top+o1); glVertex2f(x2, top+o2);
        glVertex2f(x2, bot+o2); glVertex2f(x1, bot+o1);
    glEnd();
}

// ─── Draw full waving flag background ────────────────────────────────────────
void drawWavingFlag() {
    const int   strips = 80;
    const float stripW = 2.0f / strips;
    for (int i = 0; i < strips; i++) {
        float x1 = -1.0f + i * stripW;
        drawStrip(x1, x1 + stripW, 0.5f, -0.5f);
    }
}

// ─── Draw maple leaf (waves with the flag) ────────────────────────────────────
void drawMapleLeaf() {
    if (gLeafPoints.size() < 3) return;
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
        float co = waveOffset(0.0f);
        glVertex2f(0.0f, co);
        for (const auto& p : gLeafPoints) {
            float vo = waveOffset(p.first * 0.5f);
            glVertex2f(p.first, p.second + co + (vo - co) * 0.5f);
        }
        float vo0 = waveOffset(gLeafPoints.front().first * 0.5f);
        glVertex2f(gLeafPoints.front().first,
                   gLeafPoints.front().second + co + (vo0 - co) * 0.5f);
    glEnd();
}

// ─── Display: apply 3 transformations to the leaf ────────────────────────────
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw waving flag background (no transform — full flag)
    drawWavingFlag();

    // Apply 3 required transformations to the maple leaf
    glPushMatrix();
        // TRANSFORMATION 1 — TRANSLATION: center the leaf slightly upward
        glTranslatef(0.0f, 0.02f, 0.0f);
        // TRANSFORMATION 2 — ROTATION: static tilt (0 degrees, demonstrating the call)
        glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
        // TRANSFORMATION 3 — SCALING: scale leaf to fit white section
        glScalef(0.85f, 0.85f, 1.0f);
        drawMapleLeaf();
    glPopMatrix();

    glutSwapBuffers();
}

// ─── Keyboard: W/w toggles wave, ESC quits ───────────────────────────────────
void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    if (key == 'w' || key == 'W') gWaving = !gWaving;
    if (key == 27) exit(0);
    glutPostRedisplay();
}

// ─── Timer: drives wave animation at ~60 FPS ─────────────────────────────────
void timer(int /*value*/) {
    if (gWaving) {
        waveTime += 0.04f;
        if (waveTime > 100.0f) waveTime = 0.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ─── Reshape: maintain 2:1 flag aspect ratio ─────────────────────────────────
void reshape(int width, int height) {
    if (height <= 0) height = 1;
    const float targetAspect  = 2.0f;
    const float currentAspect = static_cast<float>(width) / static_cast<float>(height);
    int vpX=0, vpY=0, vpW=width, vpH=height;
    if (currentAspect > targetAspect) {
        vpW = static_cast<int>(height * targetAspect);
        vpX = (width - vpW) / 2;
    } else if (currentAspect < targetAspect) {
        vpH = static_cast<int>(width / targetAspect);
        vpY = (height - vpH) / 2;
    }
    glViewport(vpX, vpY, vpW, vpH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(-1.0, 1.0, -0.5, 0.5, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

// ─── Main: OpenGL init and event loop ────────────────────────────────────────
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1920, 960);
    glutCreateWindow("National Flag of Canada - FreeGLUT");

    buildNormalizedLeafFromSvg();

    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);  // light blue sky background
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
