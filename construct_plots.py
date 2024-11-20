import json
import numpy as np
from matplotlib import pyplot as plt

def main():
    with open("evaluation_result.json", "r") as f:
        histories = json.load(f)

    easy_history_sum = np.array([])
    easy_history_count = 0
    medium_history_sum = np.array([])
    medium_history_count = 0
    hard_history_sum = np.array([])
    hard_history_count = 0

    for name, history in histories.items():
        if "easy" in name:
            if len(history) > len(easy_history_sum):
                easy_history_sum = np.concatenate(
                    (easy_history_sum, np.zeros(len(history) - len(easy_history_sum)))
                )
            cur_history = np.zeros(len(easy_history_sum))
            for item in history:
                cur_history[item["generation"]] = item["best_fitness"]
            easy_history_sum += cur_history
            easy_history_count += 1

        if "medium" in name:
            if len(history) > len(medium_history_sum):
                medium_history_sum = np.concatenate(
                    (medium_history_sum, np.zeros(len(history) - len(medium_history_sum)))
                )
            cur_history = np.zeros(len(medium_history_sum))
            for item in history:
                cur_history[item["generation"]] = item["best_fitness"]
            
            medium_history_sum += cur_history
            medium_history_count += 1
        
        if "hard" in name:
            if len(history) > len(hard_history_sum):
                hard_history_sum = np.concatenate(
                    (hard_history_sum, np.zeros(len(history) - len(hard_history_sum)))
                )
            cur_history = np.zeros(len(hard_history_sum))
            for item in history:
                cur_history[item["generation"]] = item["best_fitness"]
            hard_history_sum += cur_history
            hard_history_count += 1

    easy_history_avg = easy_history_sum / easy_history_count
    medium_history_avg = medium_history_sum / medium_history_count
    hard_history_avg = hard_history_sum / hard_history_count
    
    plt.figure(figsize=(10, 5))
    plt.plot(easy_history_avg[:650], label="Easy")
    plt.plot(medium_history_avg[:650], label="Medium")
    plt.plot(hard_history_avg[:650], label="Hard")
    plt.xlabel("Generation")
    plt.ylabel("AVG Fitness")
    plt.title("AVG Fitness vs Generation")
    plt.legend()
    plt.grid()
    plt.savefig("report/figures/plot.png")
    
    

if __name__ == "__main__":
    main()