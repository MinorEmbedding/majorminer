import logging
import os
import shutil
from random import random

from src.drawing.draw import DrawEmbedding
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver
from src.util.logging import init_logger

init_logger()
logger = logging.getLogger('evolution')


################################# Params #######################################

max_total = 1
max_mutations_trials = 30
population_size = 7
max_generations = 300
remove_redundant_nodes_probability = 0.1
mutation_trials_until_extend_to_free_neighbors = int(max_mutations_trials / 2)


############################### Evolution ######################################

def main_loop():
    for i in range(max_total):
        logger.info('')
        logger.info('#############')
        logger.info('🎈 NEW MAIN 🎈')
        logger.info('#############')
        logger.info('')
        logger.info(f'Calling main: {i}')

        d = DrawEmbedding(5, 5, 4)
        res = main(d)
        save_final(d)
        if res:
            break


def main(d: DrawEmbedding) -> bool:
    # logger.info('--- Main ---')

    # --- Clear
    # Clear out directory
    try:
        shutil.rmtree('./out/')
    except FileNotFoundError:
        pass
    os.mkdir('./out')

    # --- Setup
    H = TestGraph.k(12)

    solver = EmbeddingSolver(H)
    solver.initialize_embedding()
    solver.local_maximum()
    save_embedding(*solver.get_embedding(), d, -1,
                   title=f'Initial embedding')

    if solver.found_embedding():
        logger.info('🎉 Directly found embedding after initialization')
        # save_final(d)
        output_embedding(*solver.get_embedding(), d)
        return True

    # --- Start solver
    for i in range(max_generations):
        logger.info('')
        logger.info(f'🔄 Generation: {i}')

        # Generate children for one population
        population = []  # list of Embeddings
        last_trial_used = False
        for _ in range(population_size):
            logger.info('')
            logger.info(f'--- Try find a new viable mutation')
            mutation = None

            for trial, mutation in enumerate(range(max_mutations_trials)):
                # Do one mutation
                logger.info('--- MUTATION')
                mutation = solver.extend_random_supernode()
                if mutation:
                    population.append(mutation)
                    break  # try to construct next child
                elif trial >= mutation_trials_until_extend_to_free_neighbors:
                    solver.extend_random_supernode_to_free_neighbors()

            if not mutation:
                if not len(population):
                    if not last_trial_used:
                        # Before all fails: try to remove unnecessary supernode nodes
                        # and try once more
                        solver.remove_redundant_supernode_nodes()
                        logger.info(f'🔳 Last trial, remove redundant nodes')
                        continue
                    else:
                        logger.info(f'🔳 All {max_mutations_trials} mutations failed, '
                                    'could not construct a single child -> Abort')
                        return False
                else:
                    logger.info(f'🔳 {max_mutations_trials} mutations to construct '
                                ' a new child failed, will continue '
                                f'with smaller population: {len(population)}/{population_size}')
                    # break early since it is improbable that we will be able
                    # to generate more children
                    break

        # Choose best child
        improvements = [mutation.try_embed_missing_edges() for mutation in population]
        best_mutation_index = improvements.index(max(improvements))
        best_mutation = population[best_mutation_index]

        # Leave room for next generation
        if random() < remove_redundant_nodes_probability:
            logger.info('Try to remove redundant supernode nodes')
            best_mutation.remove_redundant_supernode_nodes()
        # best_mutation.remove_unnecessary_edges_between_supernodes()

        solver.commit(best_mutation)

        # Save every x generations
        save_embedding(*solver.get_embedding(), d, i,
                       title=f'Generation {i}')

        # Check if we found valid embedding
        if best_mutation.is_valid_embedding():
            logger.info('🎉🎉🎉🎉🎉🎉 Found embedding')
            return True
        else:
            logger.info('✅ Generation passed')

    return False


################################ Output ########################################

def output_embedding(nodes, edges, mapping_G_to_H, d: DrawEmbedding):
    logger.info('*** Embedding ***')
    logger.info(nodes)
    logger.info(edges)
    logger.info(mapping_G_to_H)

    d.draw_chimera_and_embedding(nodes, edges, mapping_G_to_H)
    d.show_embedding()


def save_embedding(nodes: set[int], edges: set[tuple[int, int, int]],
                   mapping_G_to_H, d: DrawEmbedding, i: int, title=''):
    logger.info('')
    logger.info('🎈 Current embedding')
    logger.info(f'edges: {edges}')
    logger.info(f'mapping_G_to_H: {mapping_G_to_H}')

    d.draw_whole_embedding_step(nodes, edges, mapping_G_to_H, title=title)
    d.save_and_clear(f'./out/{i}.svg')


def save_final(d: DrawEmbedding) -> None:
    logger.info('Save final')
    pass
    # d.save_and_clear(f'./out/steps.svg')

    ################################ Main ##########################################


if __name__ == "__main__":
    main_loop()
