# Iterated Prisoner's Dilemma Simulator

A C++ simulator for running Iterated Prisoner's Dilemma experiments across multiple strategies, payoff matrices, noise levels, and evolutionary settings.

The project is structured as a small console application. It supports baseline tournaments, pairwise payoff matrices, noisy-action experiments, exploiter tests, evolutionary population updates, and a Strategic Complexity Budget mode.

## Features

- Round-robin Iterated Prisoner's Dilemma tournaments
- Configurable match length, repeat count, random seed, payoff matrix, and strategy set
- Action noise using an epsilon probability of flipping intended moves
- Text, CSV, and JSON output modes
- Pairwise payoff matrix generation
- Strategy factory for constructing strategies from CLI tokens
- Configuration save/load support
- Evolutionary population update with mutation
- Strategic Complexity Budget scoring adjustment
- Metadata files for experiment settings

## Implemented strategies

The simulator includes the following strategy types:

| Strategy | Description |
| --- | --- |
| `ALLC` | Always cooperates |
| `ALLD` | Always defects |
| `TFT` | Tit-for-tat; cooperates first, then mirrors the opponent's previous move |
| `GRIM` | Cooperates until the opponent defects, then defects forever |
| `PAVLOV` | Win-stay, lose-shift style strategy |
| `PROBER` | Tests the opponent with an opening probe sequence, then exploits or responds reactively |
| `CTFT` / `CONTRITE` | Contrite tit-for-tat variant that tracks intended and actual moves under noise |
| `TTFT` | Two-tits-for-tat; defects only after two consecutive opponent defections |
| `GRUDGE2` | Cooperates until two opponent defections have occurred |
| `RND(p)` | Random strategy that cooperates with probability `p`, e.g. `RND0.3` or `RND(0.3)` |

Use `--list-strategies` to print the registered strategy tokens.

## Repository structure

```text
.
├── CSC8501 Prisoners Dilemna - Oliver Wilde/   # C++ source files and Visual Studio project
│   ├── main.cpp
│   ├── App.cpp / App.hpp
│   ├── Config.cpp / Config.hpp
│   ├── Match.cpp / Match.hpp
│   ├── Player.cpp / Player.hpp
│   ├── Strategy.hpp
│   ├── Strategies.cpp / Strategies.hpp
│   ├── StrategyFactory.cpp / StrategyFactory.hpp
│   ├── Tournament.cpp / Tournament.hpp
│   ├── TournamentRunner.cpp / TournamentRunner.hpp
│   ├── SweepRunner.cpp / SweepRunner.hpp
│   ├── Evolution.cpp / Evolution.hpp
│   ├── EvolutionRunner.cpp / EvolutionRunner.hpp
│   ├── SCBRunner.cpp / SCBRunner.hpp
│   ├── ResultWriters.cpp / ResultWriters.hpp
│   └── MetaWriter.cpp / MetaWriter.hpp
├── configs/
│   └── run.cfg
├── meta/                                  # Example metadata outputs
├── results/                               # Example result outputs
├── run.cfg
├── CSC8501 Prisoners Dilemma - Oliver Wilde.sln
└── REPORT OLIVER WILDE.pdf
```

## Build

The project was developed as a Visual Studio C++ console application.

### Visual Studio

1. Open `CSC8501 Prisoners Dilemma - Oliver Wilde.sln`.
2. Select an x64 configuration.
3. Build the solution.
4. Run the generated executable from a terminal to pass command-line arguments.

The project uses standard C++ and has no third-party library dependencies.

## Quick start

From the directory containing the built executable:

```bash
ipd.exe --list-strategies
```

Run the default baseline experiments:

```bash
ipd.exe --run-all
```

Run a baseline tournament with a custom strategy set:

```bash
ipd.exe --run Q1 --rounds 100 --repeats 30 --seed 1 --strategies ALLC ALLD TFT GRIM PAVLOV CTFT PROBER --format text
```

Write output to a file:

```bash
ipd.exe --run Q1 --rounds 100 --repeats 30 --seed 1 --strategies ALLC ALLD TFT GRIM PAVLOV CTFT PROBER --format text > results/q1_baseline_text.txt
```

## Experiment modes

The application supports five labelled run modes.

### Q1: Baseline tournament

Runs a round-robin tournament and prints a leaderboard. It also generates a pairwise payoff matrix.

```bash
ipd.exe --run Q1 --rounds 100 --repeats 30 --seed 1 --strategies ALLC ALLD TFT GRIM PAVLOV CTFT PROBER --format text
```

For CSV output:

```bash
ipd.exe --run Q1 --rounds 100 --repeats 30 --seed 1 --strategies ALLC ALLD TFT GRIM PAVLOV CTFT PROBER --format csv
```

When CSV mode is used, the pairwise matrix is written to:

```text
pairwise_matrix.csv
```

### Q2: Noise sweep

Runs tournaments across one or more epsilon values. Epsilon is the probability that an intended action is flipped before scoring.

```bash
ipd.exe --run Q2 --rounds 100 --repeats 30 --seed 1 --epsilons 0 0.05 0.1 0.2 --strategies TFT GRIM PAVLOV CTFT --format text
```

CSV-style epsilon input is also supported:

```bash
ipd.exe --run Q2 --epsilon 0,0.05,0.1,0.2 --strategies TFT GRIM PAVLOV CTFT --format csv
```

### Q3: Exploiter test

Runs exploiter-focused comparisons under noiseless and noisy conditions.

```bash
ipd.exe --run Q3 --rounds 100 --repeats 30 --seed 1 --epsilon 0.05 --strategies ALLC ALLD TFT PAVLOV PROBER CTFT --format text
```

### Q4: Evolutionary dynamics

Runs a replicator-style population update over repeated tournaments.

```bash
ipd.exe --run Q4 --evolve 1 --rounds 100 --generations 50 --population 200 --mutation 0.05 --strategies ALLC ALLD TFT GRIM PAVLOV CTFT PROBER --format text
```

The final population distribution is printed as counts, shares, and percentages.

### Q5: Strategic Complexity Budget

Applies per-strategy cost penalties to tournament scores.

```bash
ipd.exe --run Q5 --scb --rounds 100 --repeats 30 --seed 1 --strategies ALLC ALLD TFT GRIM PAVLOV CTFT PROBER --scb-cost ALLC=0,ALLD=0,TFT=2,GRIM=2,PAVLOV=2,CTFT=3,PROBER=3 --format text
```

Costs are interpreted as score penalties using:

```text
adjusted_score = raw_score / (1 + cost)
```

## Command-line options

| Option | Description |
| --- | --- |
| `--rounds N` | Number of rounds per match |
| `--repeats N` | Number of repeated tournaments/matches per run |
| `--seed N` | Random seed |
| `--epsilon E` | Single noise value, or CSV list of noise values |
| `--epsilons E1 E2 ...` | Multiple noise values |
| `--strategies A B ...` | Strategy tokens to include |
| `--format text\|csv\|json` | Output format |
| `--payoffs T R P S` | Payoff matrix values |
| `--evolve 0\|1` | Enables evolution-related behaviour |
| `--population N` | Population size used when displaying evolutionary results |
| `--generations G` | Number of evolutionary update generations |
| `--mutation MU` | Mutation rate in `[0, 1]` |
| `--run Q1 Q2 ...` | Selects one or more experiment modes |
| `--run-all` | Runs the default experiment sequence |
| `--save FILE` | Saves the current configuration to a file |
| `--load FILE` | Loads a configuration from a file |
| `--scb [on\|off]` | Enables or disables Strategic Complexity Budget mode |
| `--scb-cost k=v ...` | Sets per-strategy cost values |
| `--force` | Allows invalid payoff tuples |
| `--verbose` | Enables extra logging for supported modes |
| `--list-strategies` | Prints registered strategy names and exits |
| `--help` | Prints the built-in help text |

## Payoff validation

By default, custom payoff values are checked against the usual Prisoner's Dilemma constraints:

```text
T > R > P > S
2R > T + S
```

Use `--force` to bypass validation for non-standard experiments.

## Configuration files

A run can be saved:

```bash
ipd.exe --rounds 100 --repeats 10 --seed 1 --epsilon 0.03 --strategies ALLC TFT --format json --save run.cfg
```

And loaded again:

```bash
ipd.exe --load run.cfg --run Q1
```

Example configuration files are included as `run.cfg` and `configs/run.cfg`.

## Output files

The simulator primarily writes results to standard output. The repository also contains example outputs in `results/`, including:

```text
results/q1_baseline_text.txt
results/q1_baseline_csv_stdout.txt
results/q1_pairwise_matrix.csv
results/q2_epsilon_sweep.txt
results/q3_exploiter_test.txt
results/q4_evolution.txt
results/q5_scb.txt
```

Metadata JSON files are written for experiment settings and are included in `meta/`.

## Design overview

The simulator is divided into small components:

- `Strategy` defines the common strategy interface.
- `Strategies` implements concrete player strategies.
- `StrategyFactory` creates strategies from command-line tokens.
- `Player` wraps a strategy instance and forwards move/history events.
- `Match` executes repeated rounds between two players.
- `TournamentRunner` handles round-robin tournaments and pairwise matrices.
- `SweepRunner` implements Q1-Q3 experiments.
- `Evolution` and `EvolutionRunner` implement population updates.
- `SCBRunner` implements Strategic Complexity Budget adjustment.
- `Config` parses command-line arguments and configuration files.
- `ResultWriters` and `MetaWriter` handle output formatting and metadata.

## Notes

- The simulator is a console application with no GUI.
- Randomness is controlled through `--seed` for reproducible runs.
- Strategy tokens are case-insensitive.
- `RND` accepts both `RND0.3` and `RND(0.3)` forms.
