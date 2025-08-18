#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

static const int kScreenW = 1600;
static const int kScreenH = 1200;
static const float kPhysicsHz = 120.0f;
static const float kDt = 1.0f / kPhysicsHz;
static const float kLinearDamping = 0.9992f;

struct vec3d {
    float x = 0.f, y = 0.f, z = 0.f;

    vec3d() = default;
    vec3d(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    vec3d operator+(const vec3d& o) const { return {x + o.x, y + o.y, z + o.z}; }
    vec3d operator-(const vec3d& o) const { return {x - o.x, y - o.y, z - o.z}; }
    vec3d operator*(float s) const { return {x * s, y * s, z * s}; }
    vec3d operator/(float s) const { return {x / s, y / s, z / s}; }

    vec3d& operator+=(const vec3d& o){ x += o.x; y += o.y; z += o.z; return *this; }
    vec3d& operator-=(const vec3d& o){ x -= o.x; y -= o.y; z -= o.z; return *this; }
    vec3d& operator*=(float s){ x *= s; y *= s; z *= s; return *this; }
};

struct Camera {
    vec3d pos {0, 0, 300};
    vec3d target {0, 0, 0}; 
    vec3d up {0, 1, 0};
};

struct Body {
    vec3d pos, vel;
    float radius, mass, invMass;
    vec3d color;

    Body(vec3d p, vec3d v, float m, float r, vec3d c)
        : pos(p), vel(v), mass(m), radius(r), color(c)
    {
        invMass = (mass > 0.f) ? (1.f / mass) : 0.f;
    }

    void integrate(float dt){
        pos += vel * dt;
        vel *= kLinearDamping;
    }

    void draw() const {
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);

        GLfloat matColor[] = { color.x, color.y, color.z, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matColor);

        GLUquadric* quad = gluNewQuadric();
        gluSphere(quad, radius, 24, 24);
        gluDeleteQuadric(quad);

        glPopMatrix();
    }
};

struct PerpendicularOrbiter {
    float radius;           // Distance from sun
    float orbitalPeriod;    // How long one orbit takes
    float currentAngle;     // Current position (0-2π)
    float angleVelocity;    // How fast it moves
    float tiltAngle;        // How much it's tilted from the main plane
    vec3d color;
    float bodyRadius;
    std::string name;
    
    PerpendicularOrbiter(float r, float period, float tilt, vec3d col, float bodyR, std::string n) 
        : radius(r), orbitalPeriod(period), currentAngle(0.0f), tiltAngle(tilt), 
          color(col), bodyRadius(bodyR), name(n) {
        angleVelocity = 2.0f * 3.14159f / orbitalPeriod;
    }
    
    vec3d getCurrentPosition() const {
        // Calculate position in tilted orbital plane
        float x = radius * cosf(currentAngle);
        float y = radius * sinf(currentAngle) * cosf(tiltAngle);  // Projected onto main plane
        float z = radius * sinf(currentAngle) * sinf(tiltAngle);  // Perpendicular component
        return vec3d(x, y, z);
    }
    
    void update(float dt, float timeSpeed) {
        currentAngle += angleVelocity * timeSpeed * dt;
        while (currentAngle > 2.0f * 3.14159f) {
            currentAngle -= 2.0f * 3.14159f;
        }
    }
};

struct OrbitParams {
    float semiMajorAxis;    // Average distance from sun
    float eccentricity;     // How elliptical (0 = circle, 0.9 = very elliptical)
    float orbitalPeriod;    // How long one orbit takes
    float currentAngle;     // Current position in orbit (0-2π)
    float angleVelocity;    // How fast it moves around orbit
    vec3d color;
    float radius;
    std::string name;
};

// Supernova system
enum SupernovaState {
    NORMAL,
    PRIMING,      // Planets start glowing intensely
    EXPLODING,    // Massive explosions
    WHITE_FLASH,  // Screen goes white
    FADING,       // White fades to black
    ENDING        // Nothing left, program closes
};

struct SupernovaData {
    SupernovaState state;
    float timer;
    float explosionRadius;
    float whiteIntensity;
    std::vector<vec3d> explosionCenters;
    std::vector<float> explosionTimers;
    std::vector<float> explosionSizes;
    bool supernovaTriggered;
    
    SupernovaData() : state(NORMAL), timer(0.0f), explosionRadius(0.0f), 
                      whiteIntensity(0.0f), supernovaTriggered(false) {}
};
