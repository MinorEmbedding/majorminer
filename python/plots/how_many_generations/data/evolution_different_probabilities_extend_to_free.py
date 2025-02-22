# Move this file to src/solver to start

import logging
import multiprocessing
import os
import time
from functools import partial
from random import random
from typing import Optional

import numpy as np
from src.embedding.embedding import Embedding
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver, EvolutionParams
from src.util.logging import init_logger
from tqdm import tqdm

init_logger()
logger = logging.getLogger('evolution')


################################# Params #######################################

max_total = 50
max_generations = 600
remove_redundancy_probability = 0.01

# Chimera graph
m = 5  # grid size
n = 5  # grid size
t = 4  # shore size


############################### Evolution ######################################

def different_params():
    for extend_to_free_probability in np.linspace(0, 1.0, 10):
        print(f'Started with probability {extend_to_free_probability}')
        graph = TestGraph.k(8)
        start_time = time.time()
        start_multiprocessing((graph, extend_to_free_probability),
                              f'k8_extend_to_free_prob_{extend_to_free_probability}')
        duration = time.time() - start_time
        print(f'Duration using probability {extend_to_free_probability}: {duration} s')


def start_multiprocessing(plot_params, name: str):
    processes = multiprocessing.cpu_count() * int(os.getenv('CORE_PERCENTAGE', 75)) // 100
    with multiprocessing.Pool(processes) as pool:
        # Multiprocessing
        res = list(tqdm(
            pool.imap_unordered(partial(do_once, plot_params), range(max_total)),
            total=max_total)
        )

        # Save to file
        with open(f'./out/how_many_generations_{m}x{n}_{max_total}_{max_generations}_max_gen_{name}.txt', 'w') as f:
            for generations_needed in res:
                f.write(str(generations_needed) + '\n')


def do_once(plot_params, j) -> int:
    solver = EmbeddingSolver(plot_params[0], m, n, t)

    # --- Init
    solver.initialize_embedding()

    if solver.found_embedding():
        print('🎉 Directly found embedding after initialization')
        return 0

    # --- Start solver
    for i in range(max_generations):
        child = do_one_generation(i, solver, plot_params)

        if not child:
            logger.info('🔳 Stopping algorithm...')
            return -1

        solver.commit(child)

        # Check if done
        if child.is_valid_embedding():
            child.remove_redundancy()
            return i+1
        else:
            logger.info('✅ Generation passed')

    return -1


def do_one_generation(i: int, solver: EmbeddingSolver, plot_params) -> Optional[Embedding]:
    logger.info('')
    logger.info(f'🔄 Generation: {i}')

    evo_params = EvolutionParams(
        population_size=4,
        max_mutation_trials=30,
        mutation_extend_to_free_neighbors_probability=plot_params[1]  # should be <=0.5
    )

    child = solver.generate_population_and_select(evo_params)
    if not child:
        return None

    # Leave "room" on graph for next generation
    if random() < remove_redundancy_probability:
        child.remove_redundancy()

    return child


################################ Main ##########################################


if __name__ == "__main__":
    different_params()
