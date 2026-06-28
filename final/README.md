# Artificial Ecosystem Simulation

<p align="center">
  <img src="demo.gif" width="350">
</p>

<p align="center">
A real-time <b>multi-agent ecosystem simulation</b> implemented in <b>C++</b>, featuring autonomous cat behaviors, food dynamics, reproduction, adaptive population balancing, and emergent collective behavior.
</p>

---

## Project Overview

Artificial Ecosystem Simulation is a real-time grid-based ecosystem simulator where each cat behaves as an autonomous agent. The simulation models food generation, food decay, movement strategies, reproduction, aging, starvation, and adaptive population balancing to produce complex emergent behaviors.

Unlike simple cellular automata, each cat makes independent decisions based on its local environment, allowing the ecosystem to evolve dynamically over time.

---

## Demo

<p align="center">
  <img src="demo.gif" width="350">
</p>

The simulation demonstrates:

- Autonomous movement
- Food generation and decay
- Population growth
- Reproduction
- Adaptive ecosystem balancing
- Real-time terminal visualization

---

## Features

- Multi-agent ecosystem simulation
- Autonomous cat behaviors
- Food-seeking algorithm
- Herd-following behavior
- Hunger and starvation system
- Aging and growth mechanism
- Food spawning and decay
- Reproduction with environmental constraints
- Adaptive population balancing
- Multiple cat species
- ANSI color visualization
- Cross-platform terminal execution

---

## Simulation Rules

### Cat Species

| Species | Description |
|----------|-------------|
| 🔵 Blue Cat | Normal movement speed and highest starvation tolerance |
| 🔴 Red Cat | Faster movement but lower starvation tolerance |
| 🟣 Purple Cat | Juvenile cat that later develops into Blue or Red cats |

---

### Food Lifecycle

Fresh Food

↓

Stale Food

↓

Removed

Cats restore their hunger level by consuming food before it decays.

---

### Movement Strategy

Each cat determines its movement according to the following priority:

1. Search for nearby food
2. Follow neighboring cats (herding)
3. Move randomly

---

### Reproduction

Reproduction only occurs when:

- Hunger is below a threshold
- Cooldown has expired
- Nearby food exists
- Local population density is acceptable
- Compatible neighboring cats are available

Newborn cats are initially **Purple Cats**, which mature into Blue or Red cats after reaching a specific age.

---

### Population Balancing

To prevent species extinction, the simulation dynamically adjusts offspring growth probabilities according to the current population distribution.

---

## Technologies

### Language

- C++

### Programming Concepts

- Multi-Agent Systems
- Artificial Life (ALife)
- Ecosystem Simulation
- Population Dynamics
- Emergent Behavior
- Grid-based Simulation
- Object-Oriented Design
- Probabilistic Algorithms
- Real-time Rendering

### Libraries

- `<iostream>`
- `<vector>`
- `<random>`
- `<thread>`
- `<chrono>`
- `<algorithm>`

---

## Program Structure

| Function | Description |
|----------|-------------|
| `init()` | Initialize the ecosystem and generate the initial population. |
| `spawn_food()` | Generate food periodically. |
| `update_foods()` | Update food freshness and remove expired food. |
| `food_dir()` | Determine the direction toward the nearest food source. |
| `herd_dir()` | Calculate herd-following direction. |
| `move_dir()` | Decide each cat's movement strategy. |
| `build_local_map()` | Build local spatial information. |
| `scan_3x3()` | Analyze neighboring cells for reproduction. |
| `pick_birth_cell()` | Select the optimal offspring position. |
| `step_sim()` | Execute one simulation step. |
| `render()` | Render the ecosystem in real time. |

---

## Visualization

| Symbol | Meaning |
|---------|---------|
| 🔵 `#` | Blue Cat |
| 🔴 `#` | Red Cat |
| 🟣 `#` | Purple Cat |
| 🟡 `@` | Fresh Food |
| 🟤 `%` | Rotten Food |
| `.` | Empty Cell |

---

## Learning Outcomes

Through this project, I gained experience in:

- Designing autonomous agent behaviors
- Building an artificial ecosystem simulator
- Modeling population dynamics
- Designing probabilistic reproduction algorithms
- Implementing adaptive balancing strategies
- Developing real-time simulation systems
- Optimizing large-scale grid-based environments
- Creating modular C++ software architecture

---

## Future Improvements

Potential future work includes:

- Python + Pygame graphical interface
- Interactive control panel
- Real-time statistical dashboard
- Predator-prey interactions
- Seasonal weather system
- Disease transmission
- Genetic evolution
- Terrain and obstacles
- Data logging and visualization
- Parallel simulation

---

## Build and Run

### Windows

```bash
g++ Artificial_Ecosystem.c -o ecosystem.exe
ecosystem.exe
```

### Linux / macOS

```bash
g++ Artificial_Ecosystem.c -o ecosystem
./ecosystem
```

---

## Author

**Artificial Ecosystem Simulation**

A C++ multi-agent ecosystem simulator that models autonomous behaviors, food dynamics, reproduction, adaptive population balancing, and real-time ecosystem evolution.
