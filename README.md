    # DAProject2 — Register Allocation via Graph Coloring

## Overview

This project implements register allocation using interference-graph coloring.
Live ranges parsed from an input file are merged into **webs**, an interference
graph is built over those webs, and then a coloring algorithm assigns physical
registers (colors) to webs such that no two interfering webs share a register.

---

## Input Format

### Live-ranges file (`Data/ranges/rangesN.txt`)

One variable live range per line:

```
# comment — ignored
varName: p1,p2,...,pN
```

Each program point is a line number optionally annotated with `+` (definition)
or `-` (last use). Example: `i: 1+,2,3,4,5,6-`.

### Registers / config file (`Data/registers/registersN.txt`)

```
# comment — ignored
registers: N
algorithm: <variant>
```

Supported algorithm variants:

| Variant | Syntax | Meaning |
|---|---|---|
| `basic` | `algorithm: basic` | Greedy coloring, fails if graph is not K-colorable |
| `spilling` | `algorithm: spilling, K` | Greedy coloring with up to K webs spilled to memory |

---

## Algorithms

### Basic — `basicAllocation`

A Chaitin-style greedy graph-coloring allocator with **no spilling**.

**Phase 1 — Simplification**

Repeatedly scan the interference graph and remove every node whose current
degree is strictly less than `numRegisters`. Each removed node is pushed onto
a stack. When a node is removed, the degree of its neighbors is decremented,
which may make previously ineligible neighbors eligible. This repeats until
no more nodes can be removed.

If any nodes remain after simplification (every remaining node has degree ≥ K),
the graph is not K-colorable and the algorithm reports failure immediately.

**Phase 2 — Coloring**

Nodes are popped from the stack (reverse removal order) and assigned the
smallest register index not already used by any of their neighbors in the
original interference graph. Because every node was removed when its degree
was < K, a valid color is always available.

---

### Spilling — `spillingAllocation`

Extension of the basic algorithm that handles graphs that are not K-colorable
by spilling selected webs to memory (assigning them register index `-1`).

The algorithm accepts a budget `maxSpill` (the `K` parameter in the config
file). It tries to minimize the number of spills: it starts with pure coloring
and only spills a web when it is strictly necessary.

**Phase 1 — Simplification with spilling fallback**

The same degree-threshold removal loop from `basicAllocation` runs first. When
the loop gets stuck (no node has degree < `numRegisters` and the graph is not
empty), instead of failing the algorithm:

1. Checks whether the spill budget has been exhausted. If yes, reports failure.
2. Otherwise, selects the node with the **highest current degree** to spill,
   marks it as a memory operand, removes it from the graph (decrementing
   neighbor degrees), and resumes simplification.

This repeats until the graph is empty or the budget is exhausted.

**Spill heuristic — highest current degree**

The node with the most remaining interference edges is chosen for spilling.
Removing it collapses the largest number of edges in a single step, giving
the most neighbors the chance to fall below the degree threshold and become
simplifiable. This greedy strategy tends to minimize the total spills needed
by tackling the densest conflict first.

**Phase 2 — Coloring**

Identical to `basicAllocation`. Spilled nodes are not on the stack and keep
their `-1` assignment. Non-spilled nodes are colored as before.

**Result**

`result.feasible = true` whenever the remaining (non-spilled) graph was
successfully colored, even if some webs were spilled. It is `false` only when
the spill budget was exhausted and coloring is still impossible.

---

## Output Format

```
# Webs constructed with respective program points sorted in ascending order
webs: N
web0: ...
web1: ...
# Registers used, followed by assignment webs
r0 -> web0, web2
r1 -> web1
Total registers used: 2
```

If allocation is infeasible (even after exhausting the spill budget):

```
# Webs constructed with respective program points sorted in ascending order
webs: N
web0: ...
# Impossible to do with K register(s)
```

---

## Example configs (`Data/registers/`)

| File | Registers | Algorithm | Max spills | Suggested ranges file |
|---|---|---|---|---|
| `spilling_regs1.txt` | 1 | spilling | 2 | `ranges1.txt` — tight graph, needs 2 spills with 1 register |
| `spilling_regs2.txt` | 2 | spilling | 1 | `ranges2.txt` — moderate graph, 1 spill sufficient |
| `spilling_regs3.txt` | 2 | spilling | 3 | `ranges5.txt` — dense graph, up to 3 spills allowed |

---

## Usage

```
./DAProject2 <ranges_file> <registers_file> <output_file>
```

Example:

```
./DAProject2 Data/ranges/ranges1.txt Data/registers/spilling_regs1.txt Data/output/out.txt
```