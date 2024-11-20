#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>

#define SIZE 9

// reads the input sudoku from stdin
void read_puzzle(int puzzle[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char val;
            scanf(" %c", &val);
            if (val == '-' || val == '0') {
                puzzle[i][j] = 0;
            } else {
                puzzle[i][j] = val - '0';
            }
        }
    }
}

// prints formatted sudoku solution to stdout
void print_puzzle(int puzzle[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (puzzle[i][j] == 0) {
                printf("_ ");
            } else {
                if (j == SIZE - 1) {
                    printf("%d", puzzle[i][j]);
                } else {
                    printf("%d ", puzzle[i][j]);
                }
            }
        }
        printf("\n");
    }
}

// calculates the fitness of an individual
// for each row, column and 3x3 block, calculates the
// number of unique elements and evaluates
// fitness (9 - <n_unique_elems>) for each of them
// resulting fitness is the sum of all these values
int fitness(int individual[SIZE][SIZE]) {
    int score = 0;

    for (int i = 0; i < SIZE; i++) {
        int row[SIZE + 1] = { 0 };
        for (int j = 0; j < SIZE; j++) {
            row[individual[i][j]]++;
        }
        for (int k = 1; k <= SIZE; k++) {
            if (row[k] > 1) {
                score += row[k] - 1;
            }
        }
    }

    for (int j = 0; j < SIZE; j++) {
        int col[SIZE + 1] = { 0 };
        for (int i = 0; i < SIZE; i++) {
            col[individual[i][j]]++;
        }
        for (int k = 1; k <= SIZE; k++) {
            if (col[k] > 1) {
                score += col[k] - 1;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int block[SIZE + 1] = { 0 };
            for (int x = 0; x < 3; x++) {
                for (int y = 0; y < 3; y++) {
                    block[individual[3 * i + x][3 * j + y]]++;
                }
            }
            for (int k = 1; k <= SIZE; k++) {
                if (block[k] > 1) {
                    score += block[k] - 1;
                }
            }
        }
    }

    return score;
}

// creates a new individual by shuffling the numbers in each row
// and preserving fixed cells by input sudoku
void create_individual(int puzzle[SIZE][SIZE], int fixed_cells[SIZE][SIZE], int individual[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        int nums[SIZE], count = 0;
        for (int n = 1; n <= SIZE; n++) {
            int found = 0;
            for (int j = 0; j < SIZE; j++) {
                if (puzzle[i][j] == n) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                nums[count++] = n;
            }
        }

        for (int j = 0; j < SIZE; j++) {
            if (fixed_cells[i][j]) {
                individual[i][j] = puzzle[i][j];
            } else {
                int idx = rand() % count;
                individual[i][j] = nums[idx];
                nums[idx] = nums[--count];
            }
        }
    }
}

// randomly swaps two numbers in a random row
// preserving fixed cells by input sudoku
void mutate(int individual[SIZE][SIZE], int fixed_cells[SIZE][SIZE]) {
    int row = rand() % SIZE;
    int indices[SIZE], count = 0;
    for (int i = 0; i < SIZE; i++) {
        if (!fixed_cells[row][i]) {
            indices[count++] = i;
        }
    }
    if (count >= 2) {
        int i1 = rand() % count;
        int i2 = rand() % count;
        while (i1 == i2) {
            i2 = rand() % count;
        }
        int temp = individual[row][indices[i1]];
        individual[row][indices[i1]] = individual[row][indices[i2]];
        individual[row][indices[i2]] = temp;
    }
}

// takes two parents and creates a child by randomly selecting
// rows from either parent
void crossover(int parent1[SIZE][SIZE], int parent2[SIZE][SIZE], int child[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        if (rand() % 2) {
            memcpy(child[i], parent1[i], SIZE * sizeof(int));
        } else {
            memcpy(child[i], parent2[i], SIZE * sizeof(int));
        }
    }
}

// the main flow of my genetic algorithm
void genetic_algorithm(int puzzle[SIZE][SIZE], int fixed_cells[SIZE][SIZE], int population_size, int generations, int max_stagnation, double mutation_chance, int elitism_count) {
    int population[population_size][SIZE][SIZE];
    int fitness_values[population_size];

    // first, initialize population
    for (int i = 0; i < population_size; i++) {
        create_individual(puzzle, fixed_cells, population[i]);
        fitness_values[i] = fitness(population[i]);
    }

    // keep best fitness to detect stagnation
    int best_fitness = fitness_values[0];
    int stagnation_counter = 0;


    for (int generation = 0; generation < generations; generation++) {
        // sort population by fitness
        int indices[population_size];
        for (int i = 0; i < population_size; i++) {
            indices[i] = i;
        }

        std::sort(indices, indices + population_size, [&fitness_values](int a, int b) {
            return fitness_values[a] < fitness_values[b];
            });

        int sorted_population[population_size][SIZE][SIZE];
        int sorted_fitness_values[population_size];

        for (int i = 0; i < population_size; i++) {
            sorted_fitness_values[i] = fitness_values[indices[i]];
            memcpy(sorted_population[i], population[indices[i]], SIZE * SIZE * sizeof(int));
        }

        // Copy back the sorted arrays
        memcpy(fitness_values, sorted_fitness_values, population_size * sizeof(int));
        memcpy(population, sorted_population, population_size * SIZE * SIZE * sizeof(int));

        int current_fitness = fitness_values[0];

        // if found solution, print it and terminate
        if (current_fitness == 0) {
            print_puzzle(population[0]);
            return;
        }

        // update best fitness
        if (current_fitness < best_fitness) {
            best_fitness = current_fitness;
            stagnation_counter = 0;
        } else {
            stagnation_counter++;
        }

        // if too much generations showed no improvement, reset population
        if (stagnation_counter >= max_stagnation) {
            for (int i = 0; i < population_size; i++) {
                create_individual(puzzle, fixed_cells, population[i]);
                fitness_values[i] = fitness(population[i]);
            }
            best_fitness = fitness_values[0];
            stagnation_counter = 0;
        }

        int new_population[population_size][SIZE][SIZE];
        int new_fitness_values[population_size];

        // some of the best individuals remain for the next generation
        for (int i = 0; i < elitism_count; i++) {
            memcpy(new_population[i], population[i], SIZE * SIZE * sizeof(int));
            new_fitness_values[i] = fitness_values[i];
        }

        // new individuals are created by crossover of the elitism
        for (int i = elitism_count; i < population_size; i++) {
            int parent1_idx = rand() % elitism_count;
            int parent2_idx = rand() % elitism_count;
            crossover(population[parent1_idx], population[parent2_idx], new_population[i]);
            if ((double)rand() / RAND_MAX < mutation_chance) {
                mutate(new_population[i], fixed_cells);
            }
            new_fitness_values[i] = fitness(new_population[i]);
        }

        memcpy(population, new_population, population_size * SIZE * SIZE * sizeof(int));
        memcpy(fitness_values, new_fitness_values, population_size * sizeof(int));
    }

    print_puzzle(population[0]);
}

int main() {
    srand(time(NULL));

    int puzzle[SIZE][SIZE];
    read_puzzle(puzzle);

    int fixed_cells[SIZE][SIZE];
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fixed_cells[i][j] = puzzle[i][j] != 0;
        }
    }

    genetic_algorithm(puzzle, fixed_cells, 1000, 100000, 100, 0.9, 500);

    return 0;
}
