# RigidBodyLab: A Multi-threaded Game Engine Project

RigidBodyLab is an in-development multi-threaded game engine focusing on robust physics simulations and performance. At its core, it leverages advanced mathematical constructs like Vectors, Matrices, and Quaternions. The project is in its initial stages, currently featuring a suite of test cases and foundational code.

## Key Features

- Utilizes SSE2 for efficient matrix operations.
- Foundation for a multi-threaded game engine.
- Initial focus on Vector, Matrix, and Quaternion classes.
- Includes both the main project (`RigidBodyLab`) and a test suite (`RigidBodyLab_Test`).

## Building the Project

### Prerequisites

- CMake
- Visual Studio 17 2022 or later

### Automated Build Setup

A batch file named `build_project.bat` is included at the root of the project to automate the build process.

1. **Run the Batch File**

   Double-click on `build_project.bat`. This script performs the following actions:

   - Initializes and updates Git submodules.
   - Navigates to the `RigidBodyLab` directory.
   - Creates a build directory and navigates into it.
   - Runs CMake to generate the Visual Studio solution.

2. **Check the Build Output**

   After the script execution, check the `build` directory for the generated Visual Studio solution.

### Manual Build Steps

If you prefer manual setup, follow these steps:

1. **Initialize and Update Submodules**

   Navigate to the top-level folder `RigidBodyLab` and run:

   ```sh
   git submodule init
   git submodule update
   ```

2. **Build with CMake**

   In the `RigidBodyLab` directory, execute:

   ```sh
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022"
   ```

## Project Structure

- `RigidBodyLab`: The main game engine project.
- `RigidBodyLab_Test`: Contains test cases for Vector, Matrix, and Quaternion classes.

## Contributing

As this project is in its nascent stage, contributions in the form of additional features, performance improvements, or bug fixes are highly welcome.
