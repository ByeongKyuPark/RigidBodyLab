
# RigidBodyLab: Custom Physics and Graphics Engine

RigidBodyLab is a custom game engine, demonstrating an impressive blend of custom-built physics simulations and advanced graphics rendering, all crafted using OpenGL. With a solid foundation in mathematics, including Vectors, Matrices, and Quaternions, it's tailored to handle complex simulations with remarkable performance.

## Showcase
 [![RigidBodyLab Demo](https://img.youtube.com/vi/V0cV-CxENrY/0.jpg)](https://www.youtube.com/watch?v=V0cV-CxENrY)
 
Please watch the demo for a closer look at the engine's custom physics and graphics prowess.

## Key Features

- **Custom Physics and Graphics**:  Developed with a combination of custom and GLM vector and matrix operations, and OpenGL functions.
- **Advanced Graphical Techniques**: Features bump, normal, and parallax mapping for planes, translucency, refraction, and reflection for beads and mirrors.
- **Unique Mirror Implementation**: Includes both planar and spherical mirrors, with the latter using a 6-face cube map, rendering reflected images with a slight lag for a realistic effect.
- **Deferred Rendering**: This engine leverages deferred rendering, a technique that allows for efficient rendering of scenes with multiple light sources. By postponing shading and lighting until after all geometry has been processed, the engine optimizes performance and enhances the visual quality of scenes, especially in complex lighting environments.

- **Shadow Mapping**: The engine also includes an advanced shadow mapping feature, crucial for adding depth and realism to the scene. This technique helps in rendering accurate and dynamic shadows, greatly enhancing the visual realism of objects and their interactions within the environment.
- **Dynamic Object Manipulation**: Offers real-time modification of meshes and textures through a user-friendly GUI.
- **Procedural Object Generation**: Employs procedural generation for geometric modeling, adding diversity and intricacy.
- **Rigorous Physics Simulations**: Utilizes the Separating Axis Theorem (SAT) for box collisions and an impulse-based method for collision detection and resolution.
- **Intuitive Camera Controls**: Provides seamless camera movement using keyboard commands (W, A, S, D, and F).
- **Interactive Object Dynamics**: Enables launching and retrieving up to 50 varied objects at the press of a button.
- **Optimization Techniques**: Mirrors are designed to avoid mutual reflections for enhanced performance.
- **SSE2 Utilization**: Incorporates SSE2 for efficient matrix operations, a stepping stone towards a multi-threaded architecture.

## Building the Project

### Prerequisites
- CMake
- Visual Studio 17 2022 or later

### Build Options
- **Automated**: Run `build_project.bat` for a streamlined build process.
- **Manual**: Initialize Git submodules and use CMake in the `RigidBodyLab` directory.

## Project Composition
- **RigidBodyLab**: The core engine, combining physics and graphics.
- **RigidBodyLab_Test**: Tests for key mathematical components.

## Contributing
Your contributions are invaluable to the engine's evolution. Enhance features, optimize performance, or debug to help RigidBodyLab grow.

