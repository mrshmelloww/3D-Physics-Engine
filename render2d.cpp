#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;
const float G = 6.67 * pow(10, -11);

class Object {
    public:

    std::vector<float> position;
    std::vector<float> velocity;
    float radius;
    float mass;

    Object(std::vector<float> position, std::vector<float> velocity, float mass, float radius) {
        this->position = position;
        this->velocity = velocity;
        this->radius = radius;
        this->mass = mass;
    }

    void accelerate(float x, float y) {
        this->velocity[0] += x / 60.0f;
        this->velocity[1] += y / 60.0f;
    }

    void updatePos() {
        this->position[0] += this->velocity[0];
        this->position[1] += this->velocity[1];
    }

    void drawCircle(int res = 50) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2d(position[0], position[1]);
        for (int i = 0; i <= res; ++i) {
            float angle = 2.0f * M_PI * (static_cast<float>(i) / res);
            float x = position[0] + std::cos(angle) * radius;
            float y = position[1] + std::sin(angle) * radius;
            glVertex2d(x, y);
        }
        glEnd();
    }
};

GLFWwindow* StartGLFW();
void resizeUpdate(int fbW, int fbH, float &Cx, float &Cy, GLFWwindow* window);
void handleBorders(Object &obj, int fbW, int fbH);
void handleCollision(Object &a, Object &b);

int main() {
    GLFWwindow* window = StartGLFW();
    if (!window) return -1;

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);

    float centerX = fbW / 2.0f;
    float centerY = fbH / 2.0f;
    float radius = 300.0f;
    int res = 50;

    std::vector<Object> objects = {
        Object({300, 900}, {-3.0f, -3.0f}, 7.35 * pow(10, 17), 40.0f),
        Object({1300, 300}, {3.0f, 3.0f}, 7.35 * pow(10, 17), 40.0f),
        Object({800, 600}, {0.0f, 0.0f}, 7.35 * pow(10, 21), 20.0f)
    };

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto &obj : objects) {
            for (auto &obj2 : objects) {
                if (&obj2 == &obj) continue;

                float dx = obj2.position[0] - obj.position[0];
                float dy = obj2.position[1] - obj.position[1];
                float dist = sqrt(dx*dx + dy*dy);
                dist *= 75;

                std::vector<float> dir = {dx / dist, dy/ dist};

                float Gforce = (G * obj.mass * obj2.mass) / (dist * dist);

                float acc1 = Gforce / obj.mass;
                float acc2 = Gforce / obj2.mass;

                obj.accelerate(acc1 * dir[0], acc1 * dir[1]);
                obj2.accelerate(-acc2 * dir[0], -acc2 * dir[1]);
            }

            obj.updatePos();
            obj.drawCircle();

            std::cout << obj.position[0] << ' ' << obj.position[1] << '\n';

            for (int i = 0; i < objects.size(); ++i) {
                for (int j = i + 1; j < objects.size(); ++j) {
                    handleCollision(objects[i], objects[j]);
                }
            }

            handleBorders(obj, fbW, fbH);
            obj.velocity[0] *= 0.99999f;
            obj.velocity[1] *= 0.99999f;
        }

        resizeUpdate(fbW, fbH, centerX, centerY, window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

GLFWwindow* StartGLFW() {    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize glfw, panic!" << std::endl;
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Gravity Simulaiton", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)fbW, 0.0, (double)fbH, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    return window;
}

void resizeUpdate(int fbW, int fbH, float &Cx, float &Cy, GLFWwindow* window) {

    int newFbW, newFbH; 
    glfwGetFramebufferSize(window, &newFbW, &newFbH); 
    if (newFbW != fbW || newFbH != fbH) { 
        fbW = newFbW; fbH = newFbH; 
        glViewport(0, 0, fbW, fbH); 
        glMatrixMode(GL_PROJECTION); 
        glLoadIdentity(); 
        glOrtho(0.0, (double)fbW, 0.0, (double)fbH, -1.0, 1.0); 
        glMatrixMode(GL_MODELVIEW); 
        glLoadIdentity(); 
        Cx = fbW / 2.0f; 
        Cy = fbH / 2.0f;
    }
}

void handleBorders(Object &obj, int fbW, int fbH) {

    if (obj.position[1] < 0) {
        obj.position[1] = 0; 
        obj.velocity[1] *= -0.95f; 
    }

    if (obj.position[1] > fbH) {
        obj.position[1] = fbH; 
        obj.velocity[1] *= -0.95f;
    }

    if (obj.position[0] < 0) {
        obj.position[0] = 0; 
        obj.velocity[0] *= -0.65f;
    }

    if (obj.position[0] > fbW) {
        obj.position[0] = fbW; 
        obj.velocity[0] *= -0.65f;
    }

    if (obj.position[1] <= 0 && std::abs(obj.velocity[1]) < 0.5f) {
        obj.velocity[1] = 0.0f;
    }

    return;
}

void handleCollision(Object &a, Object &b) {
    float dx = b.position[0] - a.position[0];
    float dy = b.position[1] - a.position[1];
    float dist = std::sqrt(dx * dx + dy * dy);
    float minDist = a.radius + b.radius;

    if (dist < minDist && dist > 0.0f) {
        float nx = dx / dist;
        float ny = dy / dist;

        float rvx = b.velocity[0] - a.velocity[0];
        float rvy = b.velocity[1] - a.velocity[1];

        float dot = rvx * nx + rvy * ny;

        if (dot < 0) {
            float bounce = 0.12f;

            float totalMass = a.mass + b.mass;
            float impulse = (-(1 + bounce) * dot) / totalMass;

            a.velocity[0] -= impulse * b.mass * nx;
            a.velocity[1] -= impulse * b.mass * ny;
            b.velocity[0] += impulse * a.mass * nx;
            b.velocity[1] += impulse * a.mass * ny;

            float overlap = minDist - dist;
            a.position[0] -= overlap * (b.mass / totalMass) * nx;
            a.position[1] -= overlap * (b.mass / totalMass) * ny;
            b.position[0] += overlap * (a.mass / totalMass) * nx;
            b.position[1] += overlap * (a.mass / totalMass) * ny;
        }
    }
}