#define GL_SILENCE_DEPRECATION
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "assets.h"

// helper functions
static inline float radians(float deg) {return deg * 3.14159265f / 180.0f;}
static inline float degrees(float rad)  {return rad * 180.0f / 3.14159265f;}
static inline float Dot(const vec3d& a, const vec3d& b) {return a.x*b.x + a.y*b.y + a.z*b.z;}
static inline float Length(const vec3d& v) {return std::sqrt(Dot(v,v));}
static inline vec3d Normalize(const vec3d& v) {
    float len = Length(v);
    return (len > 1e-8f) ? (v / len) : vec3d(0,0,0);
}
vec3d cross(const vec3d& a, const vec3d& b) {
    return vec3d(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// function declerations
static GLFWwindow* StartGLFW();
void updatePlanetPositions(std::vector<Body>& bodies, std::vector<OrbitParams>& orbits, float dt);
void drawOrbitTrail(const std::vector<vec3d>& trail, const vec3d& color);
void drawEllipticalOrbitGuide(const OrbitParams& orbit, const vec3d& color);
void generateStars(std::vector<vec3d>& starPositions, std::vector<float>& starBrightness, int numStars);
void drawStarField(const std::vector<vec3d>& starPositions, const std::vector<float>& starBrightness);
void drawSpacetimeGrid(const std::vector<Body>& bodies);
float calculateSpacetimeCurvature(const vec3d& point, const std::vector<Body>& bodies);
void updatePerpendicularOrbiters(std::vector<Body>& bodies, std::vector<PerpendicularOrbiter>& perpOrbiters, 
                                size_t perpStartIndex, float dt, float timeSpeed);
void updateSupernova(SupernovaData& supernova, const std::vector<Body>& bodies, float dt, GLFWwindow* window);
void drawSupernovaEffects(const SupernovaData& supernova, const std::vector<Body>& bodies);
void drawWhiteFlash(float intensity);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void updateCameraVectors();  
void initCameraAnglesFromCam();

// important constants
float yaw    = -45.0f;
float pitch  = -10.0f;
float lastX  = kScreenW / 2.0f;
float lastY  = kScreenH / 2.0f;
bool  firstMouse = true;
float mouseSensitivity = 0.12f;
vec3d cameraFront; 
const vec3d worldUp = vec3d(0,0,1);
Camera cam = {vec3d(200, -250, 100), vec3d(0, 0, 0), vec3d(0, 0, 1)};
float timeSpeed = 1.0f;
bool showSpacetimeGrid = true;

int main(){

    // initializing the engine
    GLFWwindow* window = StartGLFW();
    if (!window){
        std::cerr << "Failed to init GLFW.\n";
        return -1;
    }

    ma_engine engine;
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        std::cerr << "Failed to init miniaudio\n";
        return -1;
    }

    ma_engine_play_sound(&engine, "reprise.mp3", NULL);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);

    std::vector<Body> bodies;
    bodies.push_back(Body(vec3d(0, 0, 0), vec3d(0, 0, 0), 1000.f, 20.f, vec3d(1.f, 0.95f, 0.1f)));
    
    std::vector<OrbitParams> planetOrbits = {
        {35,  0.25f, 3.5f,  0.0f, 0.0f, vec3d(0.9f, 0.8f, 0.9f), 6.0f,  "Mercury"},
        {55,  0.15f, 5.8f,  1.2f, 0.0f, vec3d(1.f, 0.95f, 0.7f), 10.0f, "Venus"},
        {75,  0.12f, 7.2f,  2.1f, 0.0f, vec3d(0.7f, 0.9f, 1.f),  11.0f, "Earth"},     
        {100, 0.18f, 10.5f, 3.8f, 0.0f, vec3d(1.0f, 0.8f, 0.7f), 8.5f,  "Mars"},
        {140, 0.35f, 15.2f, 0.5f, 0.0f, vec3d(0.9f, 0.9f, 0.8f), 4.0f,  "Ceres"},
        {180, 0.08f, 22.0f, 0.9f, 0.0f, vec3d(0.9f, 0.85f, 0.7f), 28.0f, "Jupiter"}, 
        {250, 0.10f, 35.0f, 4.5f, 0.0f, vec3d(1.0f, 0.95f, 0.8f), 25.0f, "Saturn"},
        {320, 0.06f, 48.0f, 1.7f, 0.0f, vec3d(0.8f, 0.95f, 1.0f), 20.0f, "Uranus"},
        {400, 0.04f, 65.0f, 5.2f, 0.0f, vec3d(0.75f, 0.85f, 1.0f), 19.0f, "Neptune"},
        {480, 0.25f, 85.0f, 2.8f, 0.0f, vec3d(0.95f, 0.9f, 0.85f), 3.5f,  "Pluto"},
        {550, 0.15f, 105.f, 1.1f, 0.0f, vec3d(1.0f, 0.9f, 0.8f),  5.0f,  "Eris"},
        {620, 0.30f, 125.f, 4.7f, 0.0f, vec3d(0.85f, 0.95f, 0.9f), 4.5f,  "Sedna"},
        {720, 0.20f, 150.f, 0.3f, 0.0f, vec3d(0.95f, 0.8f, 1.0f), 7.0f,  "Xerion"},
        {800, 0.12f, 180.f, 3.9f, 0.0f, vec3d(0.8f, 1.0f, 0.85f), 9.0f,  "Verdant"}
    };

    std::vector<PerpendicularOrbiter> perpendicularOrbiters;
    perpendicularOrbiters.push_back(PerpendicularOrbiter(
        150.0f, 18.0f, radians(85.0f), vec3d(1.0f, 0.85f, 0.95f), 12.0f, "Perpendis"));
    perpendicularOrbiters.push_back(PerpendicularOrbiter(
        220.0f, 28.0f, radians(70.0f), vec3d(0.8f, 0.95f, 1.0f), 8.0f, "Tilted"));

    for (const auto& perpOrb : perpendicularOrbiters) {
        vec3d pos = perpOrb.getCurrentPosition();
        bodies.push_back(Body(pos, vec3d(0,0,0), 1.0f, perpOrb.bodyRadius, perpOrb.color));
        
        std::cout << perpOrb.name << ": radius=" << perpOrb.radius 
                << ", period=" << perpOrb.orbitalPeriod 
                << ", tilt=" << degrees(perpOrb.tiltAngle) << "°\n";
    }

    SupernovaData supernova;    // end sequence

    for (auto& orbit : planetOrbits) {
        orbit.angleVelocity = 2.0f * 3.14159f / orbit.orbitalPeriod;

        float r = orbit.semiMajorAxis * (1.0f - orbit.eccentricity * cosf(orbit.currentAngle));
        vec3d pos(r * cosf(orbit.currentAngle), r * sinf(orbit.currentAngle), 0);
        
        bodies.push_back(Body(pos, vec3d(0,0,0), 1.0f, orbit.radius, orbit.color));
        
        std::cout << orbit.name << ": semi-major=" << orbit.semiMajorAxis 
                  << ", eccentricity=" << orbit.eccentricity 
                  << ", period=" << orbit.orbitalPeriod << " time units\n";
    }

    std::vector<std::vector<vec3d>> orbitTrails(bodies.size());
    int trailUpdateCounter = 0;

    std::vector<vec3d> starPositions;
    std::vector<float> starBrightness;
    generateStars(starPositions, starBrightness, 2000);

    // engine setup
    bool paused = false;
    bool showTrails = true;
    bool showOrbitGuides = false;
    int prevSpaceState = GLFW_RELEASE;
    int prevTState = GLFW_RELEASE;
    int prevGState = GLFW_RELEASE;
    int prevRState = GLFW_RELEASE;
    int prevUpState = GLFW_RELEASE;
    int prevDownState = GLFW_RELEASE;

    double prevTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_BLEND);

    GLfloat lightPos[] = {0.f, 0.f, 0.f, 1.0f};
    GLfloat lightColor[] = {1.f, 1.f, 0.8f, 1.f};
    GLfloat ambient[] = {0.1f, 0.1f, 0.15f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    glClearColor(0.01f, 0.01f, 0.03f, 1.0f);

    initCameraAnglesFromCam();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    std::cout << "\nGravity Simulator by Odai\n";
    std::cout << "=====================================\n";
    std::cout << "Controls:\n";
    std::cout << "WASD + Q/E: Move camera\n";
    std::cout << "Mouse: Look around\n";
    std::cout << "Space: Pause/Resume\n";
    std::cout << "T: Toggle orbit trails\n";
    std::cout << "G: Toggle orbit guides\n";
    std::cout << "R: Toggle spacetime grid\n";
    std::cout << "Up/Down Arrow: Speed up/slow down time\n";
    std::cout << "Shift: Fast camera movement\n";
    std::cout << "ESC: Exit\n\n";
    std::cout << "You're in for a surpise after 3 minuntes.\n";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }

        int curSpace = glfwGetKey(window, GLFW_KEY_SPACE);
        if (curSpace == GLFW_PRESS && prevSpaceState == GLFW_RELEASE) {
            paused = !paused;
            if (!paused) prevTime = glfwGetTime();
        }
        prevSpaceState = curSpace;

        int curT = glfwGetKey(window, GLFW_KEY_T);
        if (curT == GLFW_PRESS && prevTState == GLFW_RELEASE) {
            showTrails = !showTrails;
            std::cout << "Orbit trails: " << (showTrails ? "ON" : "OFF") << std::endl;
        }
        prevTState = curT;

        int curG = glfwGetKey(window, GLFW_KEY_G);
        if (curG == GLFW_PRESS && prevGState == GLFW_RELEASE) {
            showOrbitGuides = !showOrbitGuides;
            std::cout << "Orbit guides: " << (showOrbitGuides ? "ON" : "OFF") << std::endl;
        }
        prevGState = curG;

        int curR = glfwGetKey(window, GLFW_KEY_R);
        if (curR == GLFW_PRESS && prevRState == GLFW_RELEASE) {
            showSpacetimeGrid = !showSpacetimeGrid;
            std::cout << "Spacetime grid: " << (showSpacetimeGrid ? "ON" : "OFF") << std::endl;
        }
        prevRState = curR;

        int curUp = glfwGetKey(window, GLFW_KEY_UP);
        if (curUp == GLFW_PRESS && prevUpState == GLFW_RELEASE) {
            timeSpeed *= 1.5f;
            std::cout << "Time speed: " << timeSpeed << "x" << std::endl;
        }
        prevUpState = curUp;

        int curDown = glfwGetKey(window, GLFW_KEY_DOWN);
        if (curDown == GLFW_PRESS && prevDownState == GLFW_RELEASE) {
            timeSpeed /= 1.5f;
            if (timeSpeed < 0.1f) timeSpeed = 0.1f;
            std::cout << "Time speed: " << timeSpeed << "x" << std::endl;
        }
        prevDownState = curDown;

        double now = glfwGetTime();
        double frameTime = now - prevTime;
        prevTime = now;

        if (!paused) {
            frameTime = std::min(frameTime, 0.1);
            updatePlanetPositions(bodies, planetOrbits, frameTime * timeSpeed);
            
            size_t perpStartIndex = planetOrbits.size() + 1;
            updatePerpendicularOrbiters(bodies, perpendicularOrbiters, perpStartIndex, frameTime, timeSpeed);
            
            trailUpdateCounter++;
            if (trailUpdateCounter >= 3) {
                for (size_t i = 0; i < bodies.size(); ++i) {
                    orbitTrails[i].push_back(bodies[i].pos);
                    if (orbitTrails[i].size() > 800) {
                        orbitTrails[i].erase(orbitTrails[i].begin());
                    }
                }
                trailUpdateCounter = 0;
            }
        }

        float cameraSpeed = 2.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) cameraSpeed *= 4.0f;
        
        vec3d forward = cameraFront;                    
        vec3d right = Normalize(cross(forward, cam.up));
        if (Length(right) < 1e-6f) right = vec3d(1.0f, 0.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.pos += forward * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.pos -= forward * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cam.pos += right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cam.pos -= right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cam.pos -= worldUp * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cam.pos += worldUp * cameraSpeed;

        cam.target = cam.pos + forward;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        gluLookAt(cam.pos.x, cam.pos.y, cam.pos.z,
                  cam.target.x, cam.target.y, cam.target.z,
                  cam.up.x, cam.up.y, cam.up.z);

        updateSupernova(supernova, bodies, (float)frameTime, window);

        if (supernova.state == ENDING) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glfwSwapBuffers(window);
            continue;
        }

        glDisable(GL_LIGHTING);
        drawStarField(starPositions, starBrightness);
        glEnable(GL_LIGHTING);

        if (showSpacetimeGrid) {
            glDisable(GL_LIGHTING);
            drawSpacetimeGrid(bodies);
            glEnable(GL_LIGHTING);
        }

        if (showOrbitGuides) {
            glDisable(GL_LIGHTING);
            for (size_t i = 0; i < planetOrbits.size(); ++i) {
                drawEllipticalOrbitGuide(planetOrbits[i], planetOrbits[i].color * 0.3f);
            }
            glEnable(GL_LIGHTING);
        }

        if (showTrails) {
            glDisable(GL_LIGHTING);
            for (size_t i = 1; i < orbitTrails.size(); ++i) {
                if (orbitTrails[i].size() > 1) {
                    drawOrbitTrail(orbitTrails[i], bodies[i].color * 0.8f);
                }
            }
            glEnable(GL_LIGHTING);
        }

        drawSupernovaEffects(supernova, bodies);
        drawWhiteFlash(supernova.whiteIntensity);

        for (size_t i = 0; i < bodies.size(); ++i) {
            if (i > 0 && bodies[i].radius > 15.0f) {
                glDisable(GL_LIGHTING);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glPushMatrix();
                glTranslatef(bodies[i].pos.x, bodies[i].pos.y, bodies[i].pos.z);
                vec3d atmosColor = bodies[i].color * 0.8f;
                glColor4f(atmosColor.x, atmosColor.y, atmosColor.z, 0.15f);
                GLUquadric* atmosQuad = gluNewQuadric();
                gluSphere(atmosQuad, bodies[i].radius * 1.3f, 20, 20);
                gluDeleteQuadric(atmosQuad);
                glPopMatrix();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_LIGHTING);
            }
            
            bodies[i].draw();

            if (i == 0) {
                glDisable(GL_LIGHTING);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                
                float time = (float)glfwGetTime();
                for (int glow = 0; glow < 5; ++glow) {
                    glPushMatrix();
                    glTranslatef(bodies[i].pos.x, bodies[i].pos.y, bodies[i].pos.z);
                    float pulse = 0.8f + 0.3f * sinf(time * 2.0f + glow * 0.5f);
                    float glowSize = bodies[i].radius * (1.5f + glow * 0.4f) * pulse;
                    float alpha = 0.2f / (glow + 1);
                    float r = 1.0f;
                    float g = 0.9f - glow * 0.15f;
                    float b = 0.1f - glow * 0.02f;
                    
                    glColor4f(r, g, b, alpha);
                    GLUquadric* glowQuad = gluNewQuadric();
                    gluSphere(glowQuad, glowSize, 20, 20);
                    gluDeleteQuadric(glowQuad);
                    glPopMatrix();
                }
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_LIGHTING);
            }
        }

        glfwSwapBuffers(window);
    }
    
    ma_engine_uninit(&engine);
    glfwTerminate();
    return 0;
}

static GLFWwindow* StartGLFW() {
    if (!glfwInit()) return nullptr;
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_SAMPLES, 8); // anti-aliasing
    GLFWwindow* win = glfwCreateWindow(kScreenW, kScreenH, "Gravity Simulator by Odai", nullptr, nullptr);
    if (!win){ glfwTerminate(); return nullptr; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    int fbW, fbH; 
    glfwGetFramebufferSize(win, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(55.0, (double)fbW / (double)fbH, 0.5, 3000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_MULTISAMPLE);
    return win;
}

void initCameraAnglesFromCam() {
    vec3d f = Normalize(cam.target - cam.pos);
    pitch = degrees(asinf(f.z));
    yaw   = degrees(atan2f(f.y, f.x));
    updateCameraVectors();
    
    double cx, cy;
    GLFWwindow* cur = glfwGetCurrentContext();
    if (cur) {
        glfwGetCursorPos(cur, &cx, &cy);
        lastX = (float)cx; lastY = (float)cy; firstMouse = false;
    }
}

void updateCameraVectors() {
    float rYaw = radians(yaw);
    float rPitch = radians(pitch);

    cameraFront.x = cosf(rPitch) * cosf(rYaw);
    cameraFront.y = cosf(rPitch) * sinf(rYaw);
    cameraFront.z = sinf(rPitch);
    cameraFront = Normalize(cameraFront);

    vec3d right = Normalize(cross(cameraFront, worldUp));
    if (Length(right) < 1e-6f) right = vec3d(1,0,0);
    cam.up = Normalize(cross(right, cameraFront));

    cam.target = cam.pos + cameraFront;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
        return;
    }

    float xoffset = (float)(lastX - xpos);
    float yoffset = (float)(lastY - ypos);
    lastX = (float)xpos;
    lastY = (float)ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw  += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraVectors();
}

void drawOrbitTrail(const std::vector<vec3d>& trail, const vec3d& color) {
    if (trail.size() < 2) return;
    
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < trail.size(); ++i) {
        float alpha = (float)i / (float)(trail.size() - 1);
        alpha = alpha * alpha * alpha; 
    
        float shimmer = 0.8f + 0.2f * sinf((float)i * 0.1f + (float)glfwGetTime() * 3.0f);
        
        glColor4f(color.x * shimmer, color.y * shimmer, color.z * shimmer, alpha * 0.95f);
        glVertex3f(trail[i].x, trail[i].y, trail[i].z);
    }
    glEnd();
}

void drawEllipticalOrbitGuide(const OrbitParams& orbit, const vec3d& color) {
    glLineWidth(1.5f);
    
    float pulse = 0.7f + 0.3f * sinf((float)glfwGetTime() * 1.5f);
    glColor4f(color.x, color.y, color.z, 0.5f * pulse);
    
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 128; ++i) {
        float angle = (float)i * 2.0f * 3.14159f / 128.0f;
        float r = orbit.semiMajorAxis * (1.0f - orbit.eccentricity * cosf(angle));
        float x = r * cosf(angle);
        float y = r * sinf(angle);
        glVertex3f(x, y, 0);
    }
    glEnd();
}

void generateStars(std::vector<vec3d>& starPositions, std::vector<float>& starBrightness, int numStars) {
    starPositions.clear();
    starBrightness.clear();
    
    srand(42);
    
    for (int i = 0; i < numStars; ++i) {
        float distance = 800.0f + (rand() % 1000);
        float theta = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
        float phi = ((float)rand() / RAND_MAX) * 3.14159f;
        
        vec3d pos;
        pos.x = distance * sinf(phi) * cosf(theta);
        pos.y = distance * sinf(phi) * sinf(theta);
        pos.z = distance * cosf(phi);
        
        starPositions.push_back(pos);
        
        float brightness = powf((float)rand() / RAND_MAX, 1.8f);
        starBrightness.push_back(brightness);
    }
}

void drawStarField(const std::vector<vec3d>& starPositions, const std::vector<float>& starBrightness) {
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    
    float time = (float)glfwGetTime();
    
    for (size_t i = 0; i < starPositions.size(); ++i) {
        float brightness = starBrightness[i] * 1.15f;
        
        float twinkle = 0.6f + 0.4f * sinf(time * (2.0f + i * 0.01f));
        brightness *= twinkle;
        
        float colorVar = (float)(i % 100) / 100.0f;
        if (colorVar < 0.8f) {
            glColor4f(brightness, brightness, brightness, brightness);
        } else if (colorVar < 0.9f) {
            glColor4f(brightness * 0.8f, brightness * 0.9f, brightness, brightness);
        } else {
            glColor4f(brightness, brightness * 0.7f, brightness * 0.4f, brightness);
        }
        
        glVertex3f(starPositions[i].x, starPositions[i].y, starPositions[i].z);
    }
    glEnd();
}

float calculateSpacetimeCurvature(const vec3d& point, const std::vector<Body>& bodies) {
    float totalCurvature = 0.0f;
    
    for (const Body& body : bodies) {
        vec3d diff = point - body.pos;
        float distance = Length(diff);
        
        if (distance < 1e-6f) distance = 1e-6f;
        
        float maxInfluenceRadius;
        float baseMultiplier;
        float localIntensity;
        
        if (body.mass > 500.0f) {
            maxInfluenceRadius = 160.0f; 
            baseMultiplier = 20.0f;
            localIntensity = 12.0f;
        } else if (body.radius > 22.0f) {
            maxInfluenceRadius = 95.0f;
            baseMultiplier = 65.0f;
            localIntensity = 50.0f; 
        } else if (body.radius > 15.0f) { 
            maxInfluenceRadius = 75.0f;
            baseMultiplier = 60.0f;
            localIntensity = 45.0f;
        } else { 
            maxInfluenceRadius = 55.0f;
            baseMultiplier = 55.0f; 
            localIntensity = 40.0f;
        }
        
        if (distance > maxInfluenceRadius) continue;
        
        float influence = 0.0f;
        
        if (distance < maxInfluenceRadius * 0.5f) { 
            float localFalloff = 1.0f - (distance / (maxInfluenceRadius * 0.5f));
            localFalloff = localFalloff * localFalloff; 
            influence += localIntensity * localFalloff;
        }
        
        float normalizedDistance = distance / maxInfluenceRadius;
        float broadFalloff = 1.0f - normalizedDistance;
        broadFalloff = broadFalloff * broadFalloff * broadFalloff;
        
        float broadInfluence = body.mass * baseMultiplier * broadFalloff / (distance * distance + 3.0f);
        influence += broadInfluence;
        
        if (body.mass <= 500.0f && distance < 25.0f) {
            float spikeIntensity = 30.0f * expf(-distance / 8.0f);
            influence += spikeIntensity;
        }
        
        if (body.mass > 500.0f) {
            float bowlEffect = 10.0f * expf(-distance / 50.0f);
            influence += bowlEffect;
        }
        
        totalCurvature += influence;
    }
    
    return totalCurvature;
}

void drawSpacetimeGrid(const std::vector<Body>& bodies) {
    const int gridSize = 120; 
    const float gridSpacing = 15.0f; 
    const float maxCurvature = 60.0f; 
    const float baseZ = 15.0f; 
    
    float time = (float)glfwGetTime();
    float pulse = 0.85f + 0.15f * sinf(time * 0.4f);
    
    glLineWidth(1.2f);
    
    std::vector<std::vector<vec3d>> gridPoints(gridSize + 1, std::vector<vec3d>(gridSize + 1));
    std::vector<std::vector<float>> curvatures(gridSize + 1, std::vector<float>(gridSize + 1));
    
    for (int i = 0; i <= gridSize; ++i) {
        for (int j = 0; j <= gridSize; ++j) {
            float x = (i - gridSize/2) * gridSpacing;
            float y = (j - gridSize/2) * gridSpacing;
            vec3d point(x, y, baseZ);
            
            float curvature = calculateSpacetimeCurvature(point, bodies);
            curvatures[i][j] = curvature;
        }
    }
    
    std::vector<std::vector<float>> smoothedCurvatures = curvatures;
    for (int i = 1; i < gridSize; ++i) {
        for (int j = 1; j < gridSize; ++j) {
            float currentCurvature = curvatures[i][j];
            
            bool nearPlanet = false;
            float x = (i - gridSize/2) * gridSpacing;
            float y = (j - gridSize/2) * gridSpacing;
            vec3d gridPoint(x, y, baseZ);
            
            for (const Body& body : bodies) {
                if (body.mass <= 500.0f) { 
                    vec3d diff = gridPoint - body.pos;
                    float distToPlanet = Length(diff);
                    if (distToPlanet < 60.0f) { 
                        nearPlanet = true;
                        break;
                    }
                }
            }
            
            if (nearPlanet) {
                float sum = currentCurvature * 6.0f;
                sum += curvatures[i-1][j] * 0.5f;
                sum += curvatures[i+1][j] * 0.5f;
                sum += curvatures[i][j-1] * 0.5f;
                sum += curvatures[i][j+1] * 0.5f;
                smoothedCurvatures[i][j] = sum / 8.0f;
            } else {
                float sum = 0.0f;
                int count = 0;
                
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        int ni = i + di;
                        int nj = j + dj;
                        if (ni >= 0 && ni <= gridSize && nj >= 0 && nj <= gridSize) {
                            float weight = (di == 0 && dj == 0) ? 4.0f : 1.0f;
                            sum += curvatures[ni][nj] * weight;
                            count += (int)weight;
                        }
                    }
                }
                smoothedCurvatures[i][j] = sum / (float)count;
            }
        }
    }
    
    for (int i = 0; i <= gridSize; ++i) {
        for (int j = 0; j <= gridSize; ++j) {
            float x = (i - gridSize/2) * gridSpacing;
            float y = (j - gridSize/2) * gridSpacing;
            
            float curvature = smoothedCurvatures[i][j];
            
            float bend = -curvature * 3.5f;
            bend = std::max(bend, -maxCurvature);
            
            vec3d point(x, y, baseZ + bend);
            gridPoints[i][j] = point;
        }
    }
    
    for (int i = 0; i <= gridSize; ++i) {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= gridSize; ++j) {
            vec3d& point = gridPoints[i][j];
            
            float depthFromBase = baseZ - point.z;
            float normalizedDepth = depthFromBase / maxCurvature;
            normalizedDepth = std::max(0.0f, std::min(1.0f, normalizedDepth));
            
            float r, g, b;
            if (normalizedDepth < 0.3f) {
                float t = normalizedDepth / 0.3f;
                r = g = b = 0.2f + t * 0.4f; 
            } else if (normalizedDepth < 0.7f) {
                float t = (normalizedDepth - 0.3f) / 0.4f;
                r = g = b = 0.6f + t * 0.4f; 
            } else {
                float t = (normalizedDepth - 0.7f) / 0.3f;
                r = 1.0f - t * 0.2f; 
                g = 1.0f - t * 0.1f; 
                b = 1.0f; 
            }
            
            float alpha = 0.7f + normalizedDepth * 0.3f; 
            
            glColor4f(r * pulse, g * pulse, b * pulse, alpha * pulse);
            glVertex3f(point.x, point.y, point.z);
        }
        glEnd();
    }
    
    for (int j = 0; j <= gridSize; ++j) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= gridSize; ++i) {
            vec3d& point = gridPoints[i][j];
            
            float depthFromBase = baseZ - point.z;
            float normalizedDepth = depthFromBase / maxCurvature;
            normalizedDepth = std::max(0.0f, std::min(1.0f, normalizedDepth));
            
            float r, g, b;
            if (normalizedDepth < 0.3f) {
                float t = normalizedDepth / 0.3f;
                r = g = b = 0.2f + t * 0.4f;
            } else if (normalizedDepth < 0.7f) {
                float t = (normalizedDepth - 0.3f) / 0.4f;
                r = g = b = 0.6f + t * 0.4f;
            } else {
                float t = (normalizedDepth - 0.7f) / 0.3f;
                r = 1.0f - t * 0.2f;
                g = 1.0f - t * 0.1f;
                b = 1.0f;
            }
            
            float alpha = 0.7f + normalizedDepth * 0.3f;
            
            glColor4f(r * pulse, g * pulse, b * pulse, alpha * pulse);
            glVertex3f(point.x, point.y, point.z);
        }
        glEnd();
    }
    
    for (size_t bodyIdx = 0; bodyIdx < bodies.size(); ++bodyIdx) {
        const Body& body = bodies[bodyIdx];
        
        bool isPerpendicularPlanet = false;
        if (bodyIdx >= bodies.size() - 2) { 
            isPerpendicularPlanet = true;
        }
        
        if (body.mass > 0.5f && !isPerpendicularPlanet) { 
            int maxCircles;
            float baseSpacing;
            float pulseSpeed;
            float maxRadius;
            
            if (body.mass > 500.0f) { 
                maxCircles = 8;
                baseSpacing = 30.0f;
                pulseSpeed = 1.2f;
                maxRadius = 200.0f;
            } else if (body.radius > 20.0f) { 
                maxCircles = 6;
                baseSpacing = 22.0f;
                pulseSpeed = 1.8f;
                maxRadius = 120.0f;
            } else { 
                maxCircles = 4;
                baseSpacing = 18.0f;
                pulseSpeed = 2.2f;
                maxRadius = 80.0f;
            }
            
            glLineWidth(2.0f);
            
            float bodyTime = time + (float)bodyIdx * 1.3f; 
            float pulsePhase = fmodf(bodyTime * pulseSpeed, 2.0f * 3.14159f);
            
            for (int wave = 0; wave < 3; ++wave) {
                float waveOffset = (float)wave * 2.1f; 
                float currentPhase = pulsePhase + waveOffset;
                
                for (int circle = 1; circle <= maxCircles; ++circle) {
                    float baseRadius = (float)circle * baseSpacing;
                    
                    float pulseExpansion = 15.0f * sinf(currentPhase - (float)circle * 0.3f);
                    float actualRadius = baseRadius + pulseExpansion;
                    
                    if (actualRadius <= 5.0f || actualRadius > maxRadius) continue;
                    
                    glBegin(GL_LINE_STRIP);
                    
                    for (int seg = 0; seg <= 64; ++seg) {
                        float angle = (float)seg * 2.0f * 3.14159f / 64.0f;
                        vec3d circlePoint = body.pos + vec3d(actualRadius * cosf(angle), actualRadius * sinf(angle), 0);
                        
                        float curvature = calculateSpacetimeCurvature(circlePoint, bodies);
                        float bend = -curvature * 2.2f;
                        bend = std::max(bend, -maxCurvature);
                        
                        circlePoint.z = baseZ + bend;
                        
                        float waveIntensity = (sinf(currentPhase - (float)circle * 0.3f) + 1.0f) * 0.5f;
                        waveIntensity = waveIntensity * waveIntensity; 
                        
                        float distanceFade = 1.0f - (actualRadius / maxRadius);
                        float waveFade = 1.0f - ((float)wave * 0.35f);
                        
                        float alpha = 0.8f * waveIntensity * distanceFade * waveFade * pulse;
                        
                        if (body.mass <= 500.0f) { 
                            float colorPulse = 0.7f + 0.3f * waveIntensity;
                            glColor4f(
                                body.color.x * colorPulse, 
                                body.color.y * colorPulse, 
                                body.color.z * colorPulse, 
                                alpha
                            );
                        } else { 
                            float goldenPulse = 0.8f + 0.2f * waveIntensity;
                            glColor4f(
                                1.0f * goldenPulse, 
                                0.9f * goldenPulse, 
                                0.6f * goldenPulse, 
                                alpha
                            );
                        }
                        
                        glVertex3f(circlePoint.x, circlePoint.y, circlePoint.z);
                    }
                    glEnd();
                }
            }
            
            float burstPhase = fmodf(bodyTime * pulseSpeed * 0.3f, 2.0f * 3.14159f);
            if (sinf(burstPhase) > 0.95f) { 
                glLineWidth(3.0f);
                
                float burstRadius = baseSpacing * (2.0f + 3.0f * sinf(burstPhase * 4.0f));
                if (burstRadius <= maxRadius) {
                    glBegin(GL_LINE_STRIP);
                    
                    for (int seg = 0; seg <= 64; ++seg) {
                        float angle = (float)seg * 2.0f * 3.14159f / 64.0f;
                        vec3d burstPoint = body.pos + vec3d(burstRadius * cosf(angle), burstRadius * sinf(angle), 0);
                        
                        float curvature = calculateSpacetimeCurvature(burstPoint, bodies);
                        float bend = -curvature * 2.2f;
                        bend = std::max(bend, -maxCurvature);
                        
                        burstPoint.z = baseZ + bend;
                        
                        float burstAlpha = (sinf(burstPhase) - 0.95f) * 20.0f; 
                        
                        if (body.mass <= 500.0f) {
                            glColor4f(
                                body.color.x * 1.5f, 
                                body.color.y * 1.5f, 
                                body.color.z * 1.5f, 
                                burstAlpha
                            );
                        } else {
                            glColor4f(1.2f, 1.1f, 0.8f, burstAlpha);
                        }
                        
                        glVertex3f(burstPoint.x, burstPoint.y, burstPoint.z);
                    }
                    glEnd();
                }
            }
        }
    }
}

void updatePlanetPositions(std::vector<Body>& bodies, std::vector<OrbitParams>& orbits, float dt) {
    for (size_t i = 0; i < orbits.size(); ++i) {
        OrbitParams& orbit = orbits[i];
        
        float currentRadius = orbit.semiMajorAxis * (1.0f - orbit.eccentricity * cosf(orbit.currentAngle));
        
        float speedMultiplier = orbit.semiMajorAxis / currentRadius;
        speedMultiplier = powf(speedMultiplier, 1.8f); 
        
        orbit.currentAngle += orbit.angleVelocity * speedMultiplier * dt;
        
        while (orbit.currentAngle > 2.0f * 3.14159f) {
            orbit.currentAngle -= 2.0f * 3.14159f;
        }
        
        float x = currentRadius * cosf(orbit.currentAngle);
        float y = currentRadius * sinf(orbit.currentAngle);
        
        bodies[i + 1].pos = vec3d(x, y, 0);
        
        bodies[i + 1].pos.z = sinf(orbit.currentAngle * 3.0f) * 2.0f;
    }
}

void updatePerpendicularOrbiters(std::vector<Body>& bodies, std::vector<PerpendicularOrbiter>& perpOrbiters, 
                                size_t perpStartIndex, float dt, float timeSpeed) {
    for (size_t i = 0; i < perpOrbiters.size(); ++i) {
        perpOrbiters[i].update(dt, timeSpeed);
        vec3d newPos = perpOrbiters[i].getCurrentPosition();
        
        if (perpStartIndex + i < bodies.size()) {
            bodies[perpStartIndex + i].pos = newPos;
        }
    }
}

void updateSupernova(SupernovaData& supernova, const std::vector<Body>& bodies, float dt, GLFWwindow* window) {
    const float SUPERNOVA_TIME = 173.0f; 
    
    supernova.timer += dt;
    
    if (!supernova.supernovaTriggered && supernova.timer >= SUPERNOVA_TIME) {
        supernova.supernovaTriggered = true;
        supernova.state = PRIMING;
        supernova.timer = 0.0f; 
        
        std::cout << "\nCOSMIC EVENT IMMINENT!\n";
        std::cout << "All celestial bodies entering supernova phase...\n";
        
        supernova.explosionCenters.clear();
        supernova.explosionTimers.clear();
        supernova.explosionSizes.clear();
        
        for (const auto& body : bodies) {
            supernova.explosionCenters.push_back(body.pos);
            supernova.explosionTimers.push_back(0.0f);
            supernova.explosionSizes.push_back(0.0f);
        }
    }
    
    if (supernova.supernovaTriggered) {
        switch (supernova.state) {
            case PRIMING:
                if (supernova.timer > 3.0f) { 
                    supernova.state = EXPLODING;
                    supernova.timer = 0.0f;
                    std::cout << "SUPERNOVA EXPLOSIONS INITIATED!\n";
                }
                break;
                
            case EXPLODING:
                for (size_t i = 0; i < supernova.explosionTimers.size(); ++i) {
                    supernova.explosionTimers[i] += dt;
                    supernova.explosionSizes[i] = supernova.explosionTimers[i] * 150.0f; 
                }
                
                if (supernova.timer > 4.0f) { 
                    supernova.state = WHITE_FLASH;
                    supernova.timer = 0.0f;
                    supernova.whiteIntensity = 1.0f;
                    std::cout << "MAXIMUM FLASH INTENSITY REACHED!\n";
                }
                break;
                
            case WHITE_FLASH:
                if (supernova.timer > 1.5f) {
                    supernova.state = FADING;
                    supernova.timer = 0.0f;
                    std::cout << "Fading to cosmic void...\n";
                }
                break;
                
            case FADING:
                supernova.whiteIntensity = 1.0f - (supernova.timer / 3.0f); 
                if (supernova.timer > 3.0f) {
                    supernova.state = ENDING;
                    supernova.timer = 0.0f;
                    std::cout << "This universe has ended.\n";
                    std::cout << "I hope you enjoyed it! :D\n";
                }
                break;
                
            case ENDING:
                if (supernova.timer > 2.0f) { 
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                break;
                
            default:
                break;
        }
    }
}

void drawSupernovaEffects(const SupernovaData& supernova, const std::vector<Body>& bodies) {
    if (!supernova.supernovaTriggered) return;
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    switch (supernova.state) {
        case PRIMING: {
            float pulseIntensity = 0.5f + 0.5f * sinf(supernova.timer * 8.0f);
            
            for (size_t i = 0; i < bodies.size(); ++i) {
                const Body& body = bodies[i];
                
                glPushMatrix();
                glTranslatef(body.pos.x, body.pos.y, body.pos.z);
                
                for (int layer = 0; layer < 6; ++layer) {
                    float layerSize = body.radius * (2.0f + layer * 0.8f) * pulseIntensity;
                    float alpha = 0.3f / (layer + 1) * pulseIntensity;
                    
                    float timeIntensity = supernova.timer / 3.0f;
                    vec3d glowColor = body.color * (1.0f + timeIntensity * 2.0f);
                    
                    glColor4f(glowColor.x, glowColor.y, glowColor.z, alpha);
                    
                    GLUquadric* quad = gluNewQuadric();
                    gluSphere(quad, layerSize, 16, 16);
                    gluDeleteQuadric(quad);
                }
                
                glPopMatrix();
            }
            break;
        }
        
        case EXPLODING: {
            for (size_t i = 0; i < supernova.explosionCenters.size(); ++i) {
                const vec3d& center = supernova.explosionCenters[i];
                float explosionSize = supernova.explosionSizes[i];
                float explosionTime = supernova.explosionTimers[i];
                
                if (explosionSize > 0.0f) {
                    glPushMatrix();
                    glTranslatef(center.x, center.y, center.z);
                    
                    for (int ring = 0; ring < 5; ++ring) {
                        float ringSize = explosionSize * (0.6f + ring * 0.2f);
                        float alpha = std::max(0.0f, 0.8f - explosionTime * 0.3f - ring * 0.1f);
                        
                        float r = 1.0f;
                        float g = 1.0f - explosionTime * 0.2f;
                        float b = std::max(0.0f, 0.8f - explosionTime * 0.4f);
                        
                        glColor4f(r, g, b, alpha);
                        
                        GLUquadric* explosionQuad = gluNewQuadric();
                        gluSphere(explosionQuad, ringSize, 20, 20);
                        gluDeleteQuadric(explosionQuad);
                    }
                    
                    glPopMatrix();
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}

void drawWhiteFlash(float intensity) {
    if (intensity <= 0.0f) return;
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor4f(1.0f, 1.0f, 1.0f, intensity);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}