# RigidBodyLab: Advanced Physics and Graphics Engine

RigidBodyLab is an evolving game engine that excels in intricate physics simulations and advanced graphics rendering. It's built with a focus on robust mathematics, including Vectors, Matrices, and Quaternions, and is designed to handle complex simulations with high performance.

## Key Features

- **Advanced Graphical Techniques**: Implements bump, normal, and parallax mapping for planes, alongside translucency, refraction, and reflection for beads and mirrors.
- **Dynamic Object Manipulation**: Supports loading objects from text files and altering meshes and textures in real-time via a GUI.
- **Procedural Object Generation**: Geometric modeling is achieved through procedural generation, ensuring variety and complexity.
- **Physics Simulations**: Uses Separating Axis Theorem (SAT) for box collisions and an impulse-based approach for collision detection and resolution.
- **Intuitive Camera Controls**: Allows for camera rotation and positioning with keyboard inputs (W, A, S, D, and V).
- **Object Interaction**: Launch up to 50 random objects with varied shapes and textures using the spacebar. Retrieve and reload them with 'R'.
- **Optimized Rendering**: For performance efficiency, mirrors do not reflect each other's images.
- **Utilizes SSE2** for efficient matrix operations, laying the foundation for a multi-threaded engine.

## Building the Project

### Prerequisites

- CMake
- Visual Studio 17 2022 or later

### Automated Build Setup

Run `build_project.bat` to automate the build process, including Git submodule initialization, CMake configuration, and Visual Studio solution generation.

### Manual Build Steps

Alternatively, manually initialize Git submodules and use CMake to build the project in the `RigidBodyLab` directory.

## Project Structure

- `RigidBodyLab`: The main engine project, encompassing physics simulations and graphical rendering.
- `RigidBodyLab_Test`: Test suite for foundational classes like Vector, Matrix, and Quaternion.

## Contributing

Contributions are welcome to enhance features, improve performance, or fix bugs. As the project grows, diverse input is invaluable for its development.
