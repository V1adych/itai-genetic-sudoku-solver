import random
import copy
import time
import optuna
import pickle


def read_puzzle(filename):
    puzzle = []
    with open(filename, "r") as f:
        for line in f:
            row = []
            for val in line.strip().split():
                if val == "_" or val == "0":
                    row.append(0)
                else:
                    row.append(int(val))
            puzzle.append(row)
    return puzzle


def print_puzzle(puzzle):
    for row in puzzle:
        print(" ".join(str(val) if val != 0 else "_" for val in row))


def fitness(individual):
    score = 0
    # Row fitness
    for row in individual:
        score += 9 - len(set(row))
    # Column fitness
    for col in zip(*individual):
        score += 9 - len(set(col))
    # Subgrid fitness
    for i in range(3):
        for j in range(3):
            block = []
            for x in range(3):
                for y in range(3):
                    block.append(individual[3 * i + x][3 * j + y])
            score += 9 - len(set(block))
    return score


def create_individual(puzzle, fixed_cells):
    individual = []
    for i in range(9):
        nums = [n for n in range(1, 10) if n not in puzzle[i]]
        row = []
        for j in range(9):
            if fixed_cells[i][j]:
                row.append(puzzle[i][j])
            else:
                n = nums.pop(random.randint(0, len(nums) - 1))
                row.append(n)
        individual.append(row)
    return individual


def mutate(individual, fixed_cells):
    row = random.randint(0, 8)
    indices = [i for i in range(9) if not fixed_cells[row][i]]
    if len(indices) >= 2:
        i1, i2 = random.sample(indices, 2)
        individual[row][i1], individual[row][i2] = (
            individual[row][i2],
            individual[row][i1],
        )


def crossover(parent1, parent2):
    child = []
    for i in range(9):
        if random.random() < 0.5:
            child.append(copy.deepcopy(parent1[i]))
        else:
            child.append(copy.deepcopy(parent2[i]))
    return child


def genetic_algorithm(
    puzzle,
    fixed_cells,
    population_size=2000,
    generations=5000,
    max_stagnation=50,
    mutation_chance=0.9,
    elitism_count=1000,
):
    # Initialize population with fitness values
    population = [
        (create_individual(puzzle, fixed_cells), 0) for _ in range(population_size)
    ]
    # Compute fitness for initial population
    for i in range(population_size):
        population[i] = (population[i][0], fitness(population[i][0]))

    best_fitness = population[0][1]
    stagnation_counter = 0

    for generation in range(generations):
        # Sort population based on fitness
        population.sort(key=lambda ind: ind[1])
        current_fitness = population[0][1]

        if current_fitness == 0:
            # print(f"Solution found at generation {generation}")
            return True, population[0][0]

        if current_fitness < best_fitness:
            best_fitness = current_fitness
            stagnation_counter = 0
        else:
            stagnation_counter += 1

        if stagnation_counter >= max_stagnation:
            # print(
            #     f"No improvement for {max_stagnation} generations. Restarting population..."
            # )
            population = [
                (create_individual(puzzle, fixed_cells), 0)
                for _ in range(population_size)
            ]
            for i in range(population_size):
                population[i] = (population[i][0], fitness(population[i][0]))
            best_fitness = population[0][1]
            stagnation_counter = 0

        new_population = population[:elitism_count]  # Elitism

        while len(new_population) < population_size:
            parents = random.sample(population[:elitism_count], 2)
            child = crossover(parents[0][0], parents[1][0])
            if random.random() < mutation_chance:
                mutate(child, fixed_cells)
            child_fitness = fitness(child)
            new_population.append((child, child_fitness))

        population = new_population

        # if generation % 5 == 0:
        #     print(f"Generation {generation}, Best fitness: {population[0][1]}")

    # print("No solution found.")
    return False, population[0][0]


def objective(trial: optuna.Trial):
    population_size = trial.suggest_int("population_size", 100, 2500)
    max_stagnation = trial.suggest_int("max_stagnation", 5, 100)
    mutation_chance = trial.suggest_float("mutation_chance", 0.1, 1.0)
    elitism_count = trial.suggest_int("elitism_count", 10, population_size)

    puzzle = read_puzzle("input.txt")
    fixed_cells = [[cell != 0 for cell in row] for row in puzzle]

    start_time = time.time()
    success_count = 0
    for _ in range(5):
        success, solution = genetic_algorithm(
            puzzle,
            fixed_cells,
            population_size=population_size,
            generations=1000,
            max_stagnation=max_stagnation,
            mutation_chance=mutation_chance,
            elitism_count=elitism_count,
        )
        # print("Best solution:")
        # print_puzzle(solution)
        success_count += success

    return time.time() - start_time, success_count


def main():
    sampler = optuna.samplers.TPESampler(n_startup_trials=20, multivariate=True)
    study = optuna.create_study(directions=["minimize", "maximize"], sampler=sampler)
    study.optimize(objective, n_trials=50, n_jobs=6)

    with open("study.pkl", "wb") as f:
        pickle.dump(study, f)

    print("Finished optimization. Best trials:")
    for trial in study.best_trials:
        print(trial.params)
        print(trial.values)


if __name__ == "__main__":
    main()
