# Gravity Simulator: A Real-Time 3D Solar System with Spacetime Visualization

**Gravity Simulator** is a high-performance, real-time 3D solar system simulation built in **C++ with OpenGL**, featuring physically accurate orbital mechanics, spacetime curvature visualization, and an immersive supernova sequence. This project demonstrates advanced graphics programming, physics simulation, and interactive 3D rendering techniques.

![Solar System](https://img.shields.io/badge/OpenGL-3D%20Graphics-blue) ![Physics](https://img.shields.io/badge/Physics-Orbital%20Mechanics-green) ![Audio](https://img.shields.io/badge/Audio-MiniAudio-red)

---

## Core Features and Technical Highlights

### **Advanced Mathematical Physics Engine**
- **Kepler's Orbital Mechanics**: Full implementation of elliptical orbits using `r = a(1 - e*cos(θ))` with variable angular velocity
- **Gravitational Field Calculations**: Real-time computation of spacetime curvature using inverse-square law with distance-based influence zones
- **Multi-Body Gravitational Interactions**: Simultaneous N-body physics simulation calculating gravitational wells from all 14+ celestial bodies
- **Numerical Integration**: Euler method with adaptive timestep and stability controls for orbital precision
- **Spherical Coordinate Transformations**: 3D vector mathematics for camera rotations using pitch/yaw/roll calculations
- **Perpendicular Orbital Planes**: Complex 3D orbital mechanics with tilted planes using rotation matrices and trigonometric projections
- **Dynamic Time Scaling**: Variable simulation speed maintaining mathematical accuracy across different temporal scales

### **Advanced 3D Graphics**
- **Spacetime Grid Visualization**: Real-time curved spacetime mesh showing gravitational wells
- **Gravitational Wave Effects**: Pulsing concentric circles emanating from massive bodies
- **Procedural Star Field**: 2000+ twinkling stars with realistic brightness distribution
- **Atmospheric Effects**: Glowing halos around large planets with alpha blending
- **Orbit Trail System**: Dynamic particle trails following planetary paths

### **Interactive Camera System**
- **Free-Look 3D Camera**: Full 6DOF movement with mouse look controls
- **Smooth Movement**: WASD + Q/E navigation with shift-to-sprint functionality
- **Real-time Updates**: Camera vectors calculated using spherical coordinates

### **Immersive Audio Experience**
- **MiniAudio Integration**: Background music with seamless looping
- **Spatial Audio Ready**: Framework supports positional sound effects

### **Spectacular Supernova Sequence**
- **Timed Event System**: Automatic supernova trigger after 3 minutes
- **Multi-Stage Effects**: Priming → Exploding → White Flash → Fade to Black
- **Universal Destruction**: All celestial bodies participate in the finale
- **Dramatic Conclusion**: Complete universe reset with graceful program termination

---

## Technical Architecture

### **Graphics Pipeline**
- **OpenGL 3.x Core**: Hardware-accelerated rendering with depth testing
- **GLFW Window Management**: Cross-platform window creation and input handling
- **GLU Quadrics**: Efficient sphere rendering for celestial bodies
- **Blending & Lighting**: Dynamic lighting effects with proper alpha compositing

### **Mathematical Physics Engine**
- **Einstein-Inspired Spacetime Curvature**: Implements gravitational field equations with `curvature = Σ(mass × influence / distance²)` across multiple influence zones
- **Kepler's Laws Implementation**: Variable orbital speed calculation using `speed = √(GM(2/r - 1/a))` for realistic perihelion/aphelion behavior
- **Multi-Zone Gravitational Modeling**: Complex field calculations with local intensity spikes, broad influence falloffs, and exponential distance decay
- **3D Rotation Mathematics**: Spherical coordinate system with pitch/yaw transformations for camera vectors and orbital plane rotations
- **Numerical Differential Equations**: Euler integration with collision detection and stability damping factors
- **Trigonometric Orbital Mechanics**: Full 3D orbital calculations with `sin/cos` projections for tilted planes and elliptical paths
- **Vector Field Mathematics**: Real-time computation of gravitational gradients for spacetime grid deformation

### **Memory Management**
- **STL Containers**: Efficient use of `std::vector` for dynamic arrays
- **Object-Oriented Design**: Clean separation between physics bodies and visual effects
- **Resource Management**: Proper OpenGL resource allocation and cleanup

---

## Controls

| Input | Action |
|-------|--------|
| **WASD** | Move camera (forward/back/left/right) |
| **Q/E** | Move camera up/down |
| **Mouse** | Look around (first-person style) |
| **Shift** | Sprint mode (4x faster movement) |
| **Space** | Pause/Resume simulation |
| **T** | Toggle orbit trails |
| **G** | Toggle orbital guides |
| **R** | Toggle spacetime grid |
| **↑/↓** | Increase/decrease time speed |
| **ESC** | Exit program |

---

## Build and Run Instructions

### **Prerequisites**
- **C++17 or later**
- **OpenGL 3.0+** compatible graphics card
- **GLFW3** - Window management library
- **GLEW** - OpenGL extension loading
- **GLU** - OpenGL utility library
- **MiniAudio** - Audio playback (header-only)

### **Linux/macOS Build**
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libglfw3-dev libglew-dev libglu1-mesa-dev

# Compile
g++ -std=c++17 render3d.cpp -lglfw -lGLEW -lGL -lGLU -o gravity_simulator

# Run
./gravity_simulator
```

### **Windows Build (MinGW)**
```bash
# Ensure GLFW, GLEW, and GLU are installed
g++ -std=c++17 render3d.cpp -lglfw3 -lglew32 -lopengl32 -lglu32 -o gravity_simulator.exe

# Run
gravity_simulator.exe
```

### **Required Files**
- `render3d.cpp` - Main simulation code
- `assets.h` - Data structures and constants
- `reprise.mp3` - Background music (place in same directory)

---

## Project Highlights

### **Advanced Rendering Techniques**
- **Procedural Content Generation**: Algorithmically generated star field with realistic distribution
- **Multi-Pass Rendering**: Separate passes for opaque objects, transparent effects, and UI overlays
- **Dynamic Lighting**: Point light source at sun center with distance-based attenuation
- **Particle Systems**: Trail rendering using GL_LINE_STRIP with alpha gradients

### **Advanced Mathematical Modeling**
- **Spacetime Curvature Equations**: Implements `calculateSpacetimeCurvature()` with sophisticated multi-zone gravitational field modeling
- **Dynamic Orbital Velocity**: Variable speed calculations using `speedMultiplier = (a/r)^1.8` for realistic Keplerian motion
- **Complex Gravitational Wells**: Three-tier influence system with local intensity spikes, broad falloff zones, and exponential decay functions
- **Vector Calculus Implementation**: Cross products, dot products, normalization, and 3D transformations throughout the physics pipeline
- **Trigonometric Projections**: Sine/cosine calculations for elliptical orbits, perpendicular planes, and spherical coordinate conversions
- **Multi-Body Force Summation**: Real-time calculation of gravitational influences from 14+ bodies using inverse-square law variations
- **Numerical Stability**: Collision avoidance with minimum distance thresholds and mathematical singularity prevention

### **Sophisticated Physics Algorithms**
- **Elliptical Orbit Mathematics**: `r = semiMajorAxis × (1 - eccentricity × cos(angle))` with dynamic angle velocity
- **Gravitational Field Modeling**: Distance-based influence zones with `mass × baseMultiplier × falloff³ / (distance² + dampening)`
- **3D Rotation Matrices**: Perpendicular orbital planes using `tiltAngle` transformations with Y-axis projection and Z-axis perpendicular components
- **Spacetime Grid Deformation**: 120×120 grid mesh with real-time curvature calculation and smoothing algorithms for visual spacetime warping
- **Wave Propagation Physics**: Concentric gravitational wave rings with `sin(phase - distance × frequency)` calculations
- **Camera Vector Mathematics**: First-person 3D navigation using spherical coordinates: `front = [cos(pitch)cos(yaw), cos(pitch)sin(yaw), sin(pitch)]`

### **User Experience Design**
- **Intuitive Controls**: Familiar WASD + mouse control scheme
- **Visual Feedback**: Real-time display of simulation parameters and toggle states
- **Progressive Complexity**: Starts simple, builds to spectacular supernova finale
- **Educational Value**: Accurate representation of orbital mechanics and gravitational effects

---

## Technical Achievements

This project demonstrates mastery of several advanced programming and **mathematical physics** concepts:

- **3D Graphics Programming**: Complete OpenGL pipeline from vertices to pixels
- **Computational Physics**: Real-time numerical integration of differential equations and multi-body gravitational systems
- **Advanced Mathematics**: Vector calculus, trigonometry, spherical coordinates, and Einstein-inspired spacetime curvature modeling
- **Interactive Systems**: Responsive controls with smooth camera movement using rotation matrices
- **Performance Optimization**: Efficient rendering of complex mathematical calculations at 60+ FPS
- **Audio Integration**: Seamless background music with professional audio library
- **Scientific Computing**: Implementation of Kepler's laws, gravitational field equations, and orbital mechanics
- **Mathematical Modeling**: Accurate physics simulation with multi-zone influence calculations and numerical stability controls

---

## Why This Project Matters

**Gravity Simulator** isn't just a visual demo — it's a comprehensive exploration of:

- **Real-Time Graphics**: Demonstrates advanced OpenGL techniques including blending, lighting, and particle effects
- **Scientific Computing**: Accurate physics simulation with mathematical precision
- **Interactive Design**: Smooth, responsive controls that enhance user engagement
- **System Architecture**: Clean, modular code organization supporting complex features
- **Performance Engineering**: Optimized rendering pipeline maintaining 60+ FPS with heavy effects

The project showcases **production-quality engineering practices**:

- **Cross-Platform Compatibility**: Works on Windows, Linux, and macOS
- **Memory Efficient**: Careful resource management with no memory leaks
- **Extensible Architecture**: Easy to add new celestial bodies, effects, or features
- **Professional Code Quality**: Clean, well-documented, and maintainable codebase

---

## Educational Value

This simulation serves as an excellent learning tool for:

- **Orbital Mechanics**: Visualizes Kepler's laws and gravitational effects
- **Spacetime Physics**: Shows how mass curves spacetime geometry
- **3D Programming**: Complete example of OpenGL graphics pipeline
- **Game Development**: Framework applicable to space games and simulations
- **Scientific Visualization**: Techniques for displaying complex physical phenomena

---

## Experience the Simulation

**Gravity Simulator** offers a unique blend of scientific accuracy and visual spectacle. Watch as planets trace their elliptical paths through curved spacetime, observe the interplay of gravitational forces, and witness the ultimate cosmic finale after 3 minutes of peaceful observation.

*"You're in for a surprise after 3 minutes."* — The simulation

---

## Contact

For questions, suggestions, or collaboration opportunities:

- **Email**: akiradev02@icloud.com
- **LinkedIn**: https://www.linkedin.com/in/odai-alqahwaji-2bbb50304

---

**Project by Odai** - *A journey through space, time, and the beauty of gravitational physics.*
