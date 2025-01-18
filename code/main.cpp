/*
Author: Rahil Vasa
Last Date Modified: 10/05/2024
Description:
Parallel processing code for Game of Life.
*/

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <thread>
#include <atomic>

// Default values for window size, cell size, number of threads, and processing type
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int PIXEL_SIZE = 5;
int NUM_THREADS = 8;
std::string PROCESSING_TYPE = "THRD";

// Grid size variables calculated based on window dimensions and pixel size
int GRID_WIDTH = WINDOW_WIDTH / PIXEL_SIZE;
int GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SIZE;
int PITCH = GRID_WIDTH + 2;  // Padding to eliminate boundary checks

// Function Prototypes
void seedRandomGrid(std::vector<uint8_t>& grid);
void updateGridSequential(std::vector<uint8_t>& grid_current, std::vector<uint8_t>& grid_next);
void updateGridThread(std::vector<uint8_t>& grid_current, std::vector<uint8_t>& grid_next);
void updateGridOMP(std::vector<uint8_t>& grid_current, std::vector<uint8_t>& grid_next);

void seedRandomGrid(std::vector<uint8_t>& grid) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));  // Seed random number generator
    for (int y = 1; y <= GRID_HEIGHT; ++y) {  // Loop over rows
        for (int x = 1; x <= GRID_WIDTH; ++x) {  // Loop over columns
            grid[y * PITCH + x] = std::rand() % 2;  // Randomly set cell to 0 or 1
        }
    }
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "n:c:x:y:t:")) != -1) {
        switch (opt) {
            case 'n':
                NUM_THREADS = std::max(2, std::atoi(optarg));  // Set number of threads
                break;
            case 'c':
                PIXEL_SIZE = std::max(1, std::atoi(optarg));  // Set pixel size
                break;
            case 'x':
                WINDOW_WIDTH = std::atoi(optarg);  // Set window width
                break;
            case 'y':
                WINDOW_HEIGHT = std::atoi(optarg);  // Set window height
                break;
            case 't':
                PROCESSING_TYPE = optarg;  // Set processing type (SEQ, THRD, OMP)
                break;
            default:
                std::cerr << "Usage: " << argv[0]
                          << " [-n num_threads] [-c cell_size] [-x width] [-y height] [-t processing_type]\n";
                exit(EXIT_FAILURE);
        }
    }

    // Recalculate grid dimensions based on new window size and pixel size
    GRID_WIDTH = WINDOW_WIDTH / PIXEL_SIZE;
    GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SIZE;
    PITCH = GRID_WIDTH + 2;  // Update pitch with padding

    // Create SFML window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Game of Life");
    window.setFramerateLimit(60);  // Limit framerate for smoother animation

    // Initialize grids with padding
    std::vector<uint8_t> grid_current((GRID_HEIGHT + 2) * PITCH, 0);  // Current grid state
    std::vector<uint8_t> grid_next((GRID_HEIGHT + 2) * PITCH, 0);     // Next grid state

    seedRandomGrid(grid_current);  // Seed the initial grid with random values

    std::vector<uint8_t>* currentGrid = &grid_current;  // Pointer to current grid
    std::vector<uint8_t>* nextGrid = &grid_next;        // Pointer to next grid

    int generation_count = 0;  // Counter for generations
    long long delta_t = 0;     // Time accumulator

    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();  // Close window
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();  // Close on Escape key
        }

        auto start = std::chrono::high_resolution_clock::now();  // Start timing

        // Update the grid based on processing type
        if (PROCESSING_TYPE == "SEQ") {
            updateGridSequential(*currentGrid, *nextGrid);
        } else if (PROCESSING_TYPE == "THRD") {
            updateGridThread(*currentGrid, *nextGrid);
        } else if (PROCESSING_TYPE == "OMP") {
            updateGridOMP(*currentGrid, *nextGrid);
        }

        auto end = std::chrono::high_resolution_clock::now();  // End timing
        delta_t += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();  // Accumulate time

        generation_count++;  // Increment generation count
        if (generation_count == 100) {
            // Output performance data every 100 generations
            std::cout << "100 generations took " << delta_t << " microseconds with ";
            if (PROCESSING_TYPE == "SEQ")
                std::cout << "single thread." << std::endl;
            else if (PROCESSING_TYPE == "THRD")
                std::cout << NUM_THREADS << " std::threads." << std::endl;
            else if (PROCESSING_TYPE == "OMP")
                std::cout << NUM_THREADS << " OMP threads." << std::endl;
            generation_count = 0;
            delta_t = 0;  // Reset time accumulator
        }

        // Swap the grids for the next iteration
        std::swap(currentGrid, nextGrid);

        // Display the current state of the grid
        window.clear(sf::Color::Black);  // Clear window

        sf::VertexArray cells(sf::Triangles);  // Vertex array for cells

        // Loop over the grid and add alive cells to the vertex array
        for (int y = 1; y <= GRID_HEIGHT; ++y) {
            for (int x = 1; x <= GRID_WIDTH; ++x) {
                if ((*currentGrid)[y * PITCH + x]) {
                    float px = (x - 1) * PIXEL_SIZE;  // Calculate x position
                    float py = (y - 1) * PIXEL_SIZE;  // Calculate y position

                    // First triangle of the cell
                    cells.append(sf::Vertex(sf::Vector2f(px, py), sf::Color::White));
                    cells.append(sf::Vertex(sf::Vector2f(px + PIXEL_SIZE, py), sf::Color::White));
                    cells.append(sf::Vertex(sf::Vector2f(px + PIXEL_SIZE, py + PIXEL_SIZE), sf::Color::White));

                    // Second triangle of the cell
                    cells.append(sf::Vertex(sf::Vector2f(px, py), sf::Color::White));
                    cells.append(sf::Vertex(sf::Vector2f(px + PIXEL_SIZE, py + PIXEL_SIZE), sf::Color::White));
                    cells.append(sf::Vertex(sf::Vector2f(px, py + PIXEL_SIZE), sf::Color::White));
                }
            }
        }
        window.draw(cells);  // Draw all cells at once
        window.display();    // Display on screen
    }

    return 0;
}

/*
Updates the grid for the next generation using sequential processing.
Iterates over each cell and applies the Game of Life rules.

Parameters:
- grid_current: Reference to the current grid state.
- grid_next: Reference to the grid where the next state will be stored.

Returns:
- void
*/
void updateGridSequential(std::vector<uint8_t>& grid_current, std::vector<uint8_t>& grid_next) {
    for (int y = 1; y <= GRID_HEIGHT; ++y) {
        int idx = y * PITCH + 1;  // Calculate starting index for the row
        for (int x = 1; x <= GRID_WIDTH; ++x, ++idx) {
            // Count the number of alive neighbors
            int neighbors = grid_current[idx - PITCH - 1] + grid_current[idx - PITCH] + grid_current[idx - PITCH + 1]
                          + grid_current[idx - 1] + grid_current[idx + 1]
                          + grid_current[idx + PITCH - 1] + grid_current[idx + PITCH] + grid_current[idx + PITCH + 1];
            // Apply the Game of Life rules
            grid_next[idx] = (grid_current[idx]) ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
        }
    }
}

/*
Updates the grid for the next generation using multiple threads.
Divides the work among threads by splitting the grid into chunks.

Parameters:
- grid_current: Reference to the current grid state.
- grid_next: Reference to the grid where the next state will be stored.

Returns:
- void
*/
void updateGridThread(std::vector<uint8_t>& grid_current, std::vector<uint8_t>& grid_next) {
    // Calculate total number of cells
    int total_cells = GRID_HEIGHT * GRID_WIDTH;
    int cells_per_thread = total_cells / NUM_THREADS;  // Cells per thread
    int extra_cells = total_cells % NUM_THREADS;       // Extra cells to distribute

    std::vector<std::thread> threads;  // Vector to hold threads

    // Lambda function for thread work
    auto worker = [&](int start_idx, int end_idx) {
        for (int idx = start_idx; idx < end_idx; ++idx) {
            int y = idx / GRID_WIDTH + 1;           // Calculate y coordinate
            int x = idx % GRID_WIDTH + 1;           // Calculate x coordinate
            int grid_idx = y * PITCH + x;           // Calculate grid index
            // Count the number of alive neighbors
            int neighbors = grid_current[grid_idx - PITCH - 1] + grid_current[grid_idx - PITCH] + grid_current[grid_idx - PITCH + 1]
                          + grid_current[grid_idx - 1] + grid_current[grid_idx + 1]
                          + grid_current[grid_idx + PITCH - 1] + grid_current[grid_idx + PITCH] + grid_current[grid_idx + PITCH + 1];
            // Apply the Game of Life rules
            grid_next[grid_idx] = (grid_current[grid_idx]) ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
        }
    };

    int start_idx = 0;  // Starting index for each thread
    for (int i = 0; i < NUM_THREADS; ++i) {
        // Calculate end index for this thread
        int end_idx = start_idx + cells_per_thread + (i < extra_cells ? 1 : 0);
        // Create and start the thread
        threads.emplace_back(worker, start_idx, end_idx);
        start_idx = end_idx;  // Update start index for next thread
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }
}

/*
Updates the grid for the next generation using OpenMP for parallel processing.
Uses a parallel for loop to divide work among threads automatically.

Parameters:
- grid_current: Reference to the current grid state.
- grid_next: Reference to the grid where the next state will be stored.

Returns:
- void
*/
void updateGridOMP(std::vector<uint8_t>& grid_current, std::vector<uint8_t>& grid_next) {
    int total_cells = GRID_HEIGHT * GRID_WIDTH;  // Total number of cells

    // Parallel for loop with OpenMP
    #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
    for (int idx = 0; idx < total_cells; ++idx) {
        int y = idx / GRID_WIDTH + 1;           // Calculate y coordinate
        int x = idx % GRID_WIDTH + 1;           // Calculate x coordinate
        int grid_idx = y * PITCH + x;           // Calculate grid index
        // Count the number of alive neighbors
        int neighbors = grid_current[grid_idx - PITCH - 1] + grid_current[grid_idx - PITCH] + grid_current[grid_idx - PITCH + 1]
                      + grid_current[grid_idx - 1] + grid_current[grid_idx + 1]
                      + grid_current[grid_idx + PITCH - 1] + grid_current[grid_idx + PITCH] + grid_current[grid_idx + PITCH + 1];
        // Apply the Game of Life rules
        grid_next[grid_idx] = (grid_current[grid_idx]) ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
    }
}
