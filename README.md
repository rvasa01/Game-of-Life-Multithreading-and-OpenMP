# John Conway's Game of Life

This project is an implementation of **John Conway's Game of Life**. It explores the use of **multithreading** with `std::thread` and **OpenMP** to handle parallel computations in this computationally intensive problem.

## Features
- **Game Objective**: Simulate generations of Conway's Game of Life based on predefined rules.
- **Rules**:
  - A live cell remains alive if it has 2 or 3 live neighbors.
  - A dead cell becomes alive if it has exactly 3 live neighbors.
- **Graphics**:
  - A 2D grid displays alive cells as white and dead cells as black.
  - The grid dynamically updates each generation.
- **Console Output**:
  - Displays the time taken (in microseconds) to compute the last 100 generations for each processing type.

## Technical Details
- **Command-Line Arguments**:
  - `-n`: Number of threads (`>= 2`).
  - `-c`: Cell size (square cells, default is 5).
  - `-x`: Window width (default is 800).
  - `-y`: Window height (default is 600).
  - `-t`: Processing type (`SEQ`, `THRD`, or `OMP`).
  - Example: `./Lab2 -n 8 -c 5 -x 800 -y 600 -t OMP`
- **Processing Types**:
  - Sequential (`SEQ`)
  - Multithreaded using `std::thread` (`THRD`)
  - Multithreaded using OpenMP (`OMP`)
- **Default Parameters**:
  - Threads: 8 (ignored for `SEQ` processing type).
  - Cell Size: 5.
  - Window Size: 800x600.
  - Processing Type: `THRD`.
- **Core Functions**:
  - **Sequential Processing**: Basic single-threaded computation.
  - **Multithreaded Processing**: Parallel computation using `std::thread`.
  - **OpenMP Processing**: Optimized parallel computation using OpenMP.
- **Random Initialization**:
  - Each cell is randomly initialized as alive or dead.

## How to Run
1. Clone the repository and compile the project using the provided `CMakeLists.txt`.
2. Run the executable with your desired command-line arguments.
3. View the simulation in the graphical window.
4. Press `Esc` to exit the application.
