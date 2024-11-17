# Epidemic Simulation Project

## Overview

This project simulates the spread of an infectious disease in a population, modeling the movement and interaction of individuals. It tracks infection, recovery, and immunity over discrete time units and implements both serial and parallel versions of the simulation. The parallel versions use OpenMP to explore different parallelization strategies for performance optimization in large-scale scenarios.

### Objective

- Simulate the evolution of a simplified infectious disease model.
- Provide both serial and two distinct parallel implementations using OpenMP.
- Ensure correctness of results between serial and parallel versions through automatic validation.
- Analyze the speedup achieved by parallelization.

---

## Project Structure

### Core Files

1. **`epidemy.c`**: Contains the shared simulation logic for moving individuals, spreading infections, and updating statuses.
2. **`epidemy.h`**: Header file defining the `Person_t` structure and function prototypes.
3. **`serial_epidemy.c`**: Implements the sequential simulation version.
4. **`parallel_epidemy.c`**: Implements two parallelized simulation versions:
   - **V1**: Uses OpenMP `parallel for` with scheduling and chunk size optimizations.
   - **V2**: Explores manual data partitioning or task-based parallelism with OpenMP.
5. **`validate_outputs.c`**: Compares results between serial and parallel executions to ensure correctness.

---

## Simulation Description

The simulation predicts the spread of an infectious disease by modeling the behavior of individuals in a bounded environment. It tracks their movement, interactions, infection states, and recovery over time. Each individual has attributes like:
- **Position**: `(x, y)` coordinates.
- **Status**: Susceptible, infected, or immune.
- **Infection counter**: Tracks the number of times the individual has been infected.

At the end of the simulation, the final state of all individuals is recorded, including their:
- Final coordinates.
- Final status.
- Total infections during the simulation.

---

## Parallel Implementations

### Version 1: OpenMP `parallel for`
- Utilizes parallel loops for simulation tasks such as updating movement and infection states.
- Investigates scheduling strategies (`static`, `dynamic`, `guided`) and varying chunk sizes to optimize load balancing.

### Version 2: Manual Partitioning / OpenMP Tasks
- Explores explicit data partitioning or task-based parallelism for more granular control over workload distribution.

Both parallel versions aim to produce the same results as the serial version while significantly reducing runtime.

---

## Features

- **Intermediate and Final Output**:
  - Debug mode outputs the state of the simulation after each time step.
  - Normal mode records only the final state, suitable for performance analysis.
- **Validation**:
  - Automatic verification ensures consistency between serial and parallel outputs.
- **Performance Measurement**:
  - Tracks runtime for serial and parallel versions and computes the speedup.

---

## Conclusion

This project leverages OpenMP to enhance the efficiency of computational simulations, providing insights into:
- Effective parallel programming techniques.
- The trade-offs in scheduling and load balancing.
- Validation methods for parallel computing.

It serves as a foundation for exploring more complex simulations and advanced parallelism techniques.
