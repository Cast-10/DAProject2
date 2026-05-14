    # DAProject2 — Register Allocation via Graph Coloring

## Overview

This project implements register allocation using interference-graph coloring.
Live ranges parsed from an input file are merged into **webs**, an interference
graph is built over those webs, and then a coloring algorithm assigns physical
registers (colors) to webs such that no two interfering webs share a register.

Three allocation strategies are provided, in increasing order of capability:
**basic** (pure coloring), **spilling** (coloring with memory fallback), and
**splitting** (coloring with live-range splitting).

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
| `spilling` | `algorithm: spilling, K` | Greedy coloring; spills up to K webs to memory when stuck |
| `splitting` | `algorithm: splitting, K` | Greedy coloring; splits up to K webs when stuck |
| `free` | `algorithm: free` | DSATUR coloring — picks vertex with highest saturation at each step |

---

## Graph Data Structure (`DataStructures/Graph.h`)

The project uses an upgraded `Graph<T>` implementation based on the DA 2024/2025
template. The key additions and changes relative to the original course skeleton
are listed below.

### Edge representation

Edges are stored as **heap-allocated pointers** (`vector<Edge<T>*>`) rather than
inline values. This allows edges to carry back-pointers (`orig`, `reverse`) and
supports bidirectional-edge pairs needed for flow and coloring algorithms.

### New `Vertex` fields

| Field | Type | Purpose |
|---|---|---|
| `dist` | `double` | General-purpose distance/priority field. Used by Dijkstra and — negated — as the priority key for DSATUR (see below). |
| `path` | `Edge<T>*` | Back-pointer for shortest-path reconstruction. |
| `incoming` | `vector<Edge<T>*>` | List of incoming edges; maintained automatically by `addEdge` / `deleteEdge`. |
| `satur` | `int` | **Saturation degree** — number of distinct colors already assigned to colored neighbors. Used exclusively by `freeAllocation` (DSATUR). Initialized to 0 before each run. |
| `queueIndex` | `int` | Heap position index required by `MutablePriorityQueue`. Managed automatically by the queue. |
| `indegree` | `unsigned int` | Degree in the interference graph, used as a tie-breaker in `operator<`. Initialized to `0` by default to avoid undefined behaviour before `setIndegree` is called. |

### `operator<` — ordering for `MutablePriorityQueue`

```cpp
bool Vertex<T>::operator<(Vertex<T>& other) const {
    if (satur == other.satur)
        return indegree > other.indegree;   // higher degree → higher priority (tie-break)
    return satur > other.satur;             // higher saturation → higher priority
}
```

The comparison is **inverted** relative to the usual `<` so that
`MutablePriorityQueue::extractMin()` always returns the vertex with the
**highest** saturation (and, on ties, the highest degree) — exactly what DSATUR
requires. This means the queue acts as a max-saturation priority queue despite
being implemented as a min-heap.

### New `Graph` members

| Member | Purpose |
|---|---|
| `addBidirectionalEdge(u, v, w)` | Adds edges in both directions and links them via `setReverse`, enabling reverse-edge traversal. |
| `distMatrix`, `pathMatrix` | Pre-allocated 2-D arrays for Floyd-Warshall; managed by the destructor. |
| `~Graph()` | Destructor that frees `distMatrix` and `pathMatrix` with `deleteMatrix`. |

---

## Building Webs

Before any allocation, live ranges are merged into **webs**.

Two live ranges of the same variable are merged into a single web if they
**overlap** (share at least one line number) or are **adjacent** (one ends
with `-` and the other begins with `+` on the same line number, i.e. a
definition immediately follows a last use at the same point).

Merging is transitive: if A overlaps B and B overlaps C, all three become one
web. After all merges, each web covers the union of its constituent ranges'
program points.

**Interference** between two webs: two webs interfere if they share at least
one program point, with one exception — if one web's annotation at that point
is `+` (definition) and the other's is `-` (last use), they do **not**
interfere (the use occurs before the definition at that program point).

---

## Algorithms

### Basic — `basicAllocation`

A Chaitin-style greedy graph-coloring allocator with **no spilling or splitting**.

**Phase 1 — Simplification**

Seed a BFS queue with every node whose current degree is strictly less than
`numRegisters` (K). Mark those nodes as removed, push them onto a stack, and
decrement the degree of each of their non-removed neighbors. When a neighbor's
degree drops below K it is also removed and enqueued. Repeat until the queue
is empty.

If any node was never removed (its degree never fell below K), the graph is not
K-colorable and the algorithm reports failure immediately — **no register
assignment is produced**.

**Phase 2 — Coloring**

Pop nodes from the stack (reverse removal order) and assign each the smallest
register index not already used by any of its neighbors in the **original**
interference graph (before removal). Because every node was removed when its
degree was < K, at least one color is always available.

**Complexity:** O(N + E) where N = number of webs, E = number of interference
edges.

**Result:** `feasible = true` and `registersUsed = max_color + 1` if the graph
was K-colorable; `feasible = false` otherwise.

---

### Spilling — `spillingAllocation`

Extension of `basicAllocation` that handles graphs that are **not K-colorable**
by sending selected webs to memory (assigning them register index `-1`).

The algorithm accepts a budget `maxSpill` (the K parameter). It tries to
minimize the number of spills: it starts with pure coloring and only spills a
web when strictly necessary.

**Phase 1 — Simplification with spilling fallback**

Run the same degree-threshold removal loop from `basicAllocation`. When the
loop gets **stuck** (no remaining node has degree < `numRegisters` and the
graph is not empty):

1. If the spill budget is exhausted (`spillCount >= maxSpill`), report failure.
2. Otherwise, select the **highest-degree** remaining node, mark it as spilled
   (assignment `-1`), remove it from the working graph (decrementing neighbor
   degrees), and resume simplification.

Repeat until the graph empties or the budget is exhausted.

**Spill heuristic — highest current degree**

The node with the most remaining interference edges is chosen. Removing it
collapses the largest number of edges at once, giving the most neighbors the
chance to fall below K and become simplifiable. This greedy strategy tends to
minimize the total number of spills needed.

**Phase 2 — Coloring**

Identical to `basicAllocation`. Spilled nodes are not on the stack and keep
their `-1` assignment. When computing forbidden colors for a non-spilled node,
spilled neighbors are ignored (they have no register to conflict with).

**Result:** `feasible = true` whenever the remaining (non-spilled) graph was
successfully K-colored, even if some webs were spilled. `feasible = false`
only when the budget is exhausted and the graph is still uncolorable.

---

### Splitting — `splittingAllocation`

Extension of `basicAllocation` that handles graphs that are **not K-colorable**
by **splitting** selected webs into two shorter halves, thereby reducing
interference edges.

The algorithm accepts a budget `maxSplit` (the K parameter). It starts with
zero splits and only increases the split count when necessary.

**How splitting works**

Splitting web W at program point P divides W into two new webs:

- **W.1** — all points of W up to and including P; P's annotation is set to
  `-` (last use / save to memory).
- **W.2** — all points of W from P onwards; P's annotation is set to `+`
  (definition / reload from memory).

By the non-interference rule, W.1 and W.2 **never interfere with each other**:
at the shared point P, W.1 carries `-` (last use) and W.2 carries `+`
(definition), which is exactly the exception case.

Other webs that only overlapped with one half of W now no longer need a
different register from the other half, potentially reducing the chromatic
number of the graph.

**Main loop (incremental)**

```
for splitsUsed in 0 .. maxSplit:
    reindex webs (ids must equal vector positions)
    rebuild interference graph
    try basicAllocation
    if feasible → return result
    if splitsUsed == maxSplit → return infeasible
    pick the best web to split and the best split point
    replace the web with its two halves
```

The algorithm starts by trying basic coloring with no splits (iteration 0).
Each subsequent iteration adds exactly one more split on top of all previous
ones, so the total number of live-range splits never exceeds `maxSplit`.

**Web selection heuristic — highest current degree**

The splittable web (≥ 2 program points) with the **most interference edges**
in the current graph is chosen. This is the web most likely to be causing
infeasibility: it blocks the most neighbors from becoming simplifiable.

**Split-point selection — minimise max half-degree**

For a web with N program points there are N−1 candidate split positions.
Every position is tried: two temporary half-webs are built and their
interference with every other web is counted. The split position that
minimises `max(degree(W.1), degree(W.2))` is selected. This tends to produce
the most balanced reduction in interference, giving the best chance that the
resulting graph is K-colorable.

**After each split**, the webs vector is re-indexed (IDs reset to 0…n−1) and
the interference graph is rebuilt from scratch before the next coloring attempt.

**Result:** same semantics as `basicAllocation` — `feasible = true` if
K-coloring succeeded (with some webs split); `feasible = false` if the budget
was exhausted and the graph remained uncolorable. Split webs appear in the
output as `webX.1` and `webX.2`.

---

### Free — `freeAllocation` (DSATUR)

A register allocator based on the **DSATUR** (Degree of SATURation) graph-coloring
algorithm, introduced by Brélaz (1979). Unlike the Chaitin-style approach used
by the other three algorithms, DSATUR does **not** pre-compute a simplification
order. Instead it colors one vertex at a time, always choosing the vertex that is
most constrained at that moment.

**Saturation degree**

The *saturation degree* of an uncolored vertex is the number of **distinct
colors** already assigned to its neighbors. A vertex with higher saturation has
fewer color choices remaining and is therefore more likely to cause a conflict if
left for later — so it is colored first.

**Algorithm**

```
Initialize every vertex with satur = 0 and indegree = degree in the graph.
Insert all vertices into a max-saturation priority queue (MutablePriorityQueue).

while queue is not empty:
    v ← extract vertex with highest saturation (ties broken by degree)
    c ← smallest color index not used by any already-colored neighbor of v
    if no such c exists within [0, numRegisters):
        report infeasible
    assign color c to v
    for each uncolored neighbor u of v:
        if c is not yet recorded as forbidden for u:
            mark c as forbidden for u
            increment satur(u)
            call decreaseKey(u)   // re-position u in the priority queue

report feasible; registersUsed = max color used + 1
```

**Priority queue**

The `MutablePriorityQueue<Vertex<int>>` is a binary min-heap that orders
vertices by `operator<`. Because `operator<` is defined so that *higher*
saturation compares as *smaller*, `extractMin` always yields the most-saturated
vertex. When a neighbor's saturation increases after a coloring step,
`decreaseKey` (which calls `heapifyUp`) correctly repositions it toward the
root.

The initial ordering before any vertex is colored uses `indegree` as a
tie-breaker (all saturations start at 0), matching the standard DSATUR
initialization that prefers high-degree vertices first.

**Forbidden-color tracking**

A `colors[webId][colorIdx]` boolean matrix records which colors are already
present in each vertex's neighborhood. This lets the inner loop find the
smallest available color in O(K) time without re-scanning neighbors.

**Complexity**

- Each vertex is extracted once: O(N log N).
- Each edge is visited twice (once per endpoint): O(E log N) for the
  `decreaseKey` calls.
- Color scan per vertex: O(K).
- Overall: **O((N + E) log N + N·K)**.

**Comparison with basic**

| Property | basic (Chaitin) | free (DSATUR) |
|---|---|---|
| Order determined | before coloring (simplification stack) | dynamically, one vertex at a time |
| Handles any K-colorable graph | yes (if degree < K condition met) | yes |
| No spilling / splitting | no fallback | no fallback |
| Uses `MutablePriorityQueue` | no | yes |
| Optimality | not guaranteed | not guaranteed, but empirically uses fewer colors on average |

**Result:** `feasible = true` and `registersUsed = max_color + 1` if every web
received a color within `[0, numRegisters)`; `feasible = false` if any vertex
had all K colors forbidden by its neighbors.

---

## Output Format

### Feasible — no spills (basic or splitting)

```
# Webs constructed with respective program points sorted in ascending order
webs: N
web0: 1+,2,3,4,5,6-
web1: 9+,10,11,12-,13,14-,20+
web2: 7+,8,9,10-
# Registers available: K
# Registers used, followed by assignment webs using basic algorithm
r0 -> web0, web2
r1 -> web1
# Total registers used: 2
```

### Feasible — splitting (some webs were split)

Split webs are listed with `.1` / `.2` suffixes everywhere they appear:

```
# Webs constructed with respective program points sorted in ascending order
webs: 4
web0: 1+,2,3,4,5,6-
web1: 9+,10,11,12-,13,14-,20+
web2.1: 7+,8,9-
web2.2: 9+,10-
# Registers available: K
# Registers used, followed by assignment webs using splitting algorithm
r0 -> web0, web2.2
r1 -> web1, web2.1
# Total registers used: 2
```

### Feasible — spilling (some webs sent to memory)

```
# Webs constructed with respective program points sorted in ascending order
webs: 3
web0: 1+,2,3,4,5,6-
web1: 9+,10,11,12-,13,14-,20+
web2: 7+,8,9,10-
# Registers available: 1
# Registers used, followed by assignment webs using spilling algorithm
# web1 was spilled to memory
# web2 was spilled to memory
r0 -> web0
M -> web1, web2
# Total registers used: 1
```

### Infeasible

```
# Webs constructed with respective program points sorted in ascending order
webs: N
web0: ...
# Registers available: K
# Registers used, followed by assignment webs using <algorithm> algorithm
Register allocation infeasible with K registers
```

---

## Example configs (`Data/registers/`)

| File                  | Registers | Algorithm | Parameter | Suggested ranges file                          |
|-----------------------|---|---|---|------------------------------------------------|
| `registers1.txt`      | 1 | basic | — | `ranges4.txt  — needs 1 register               |
| `registers2.txt`      | 2 | basic | — | `ranges3.txt` — sequential, 2-colorable        |
| `registers3.txt`      | 3 | basic | — | `ranges6.txt` — needs 3 registers              |
| `spilling_regs1.txt`  | 1 | spilling | 2 | `ranges7.txt` — K3 triangle, needs exactly 2 spills with 1 register (budget fully used) |
| `spilling_regs2.txt`  | 2 | spilling | 1 | `ranges6.txt` — 5-cycle, needs exactly 1 spill with 2 registers (budget fully used)     |
| `spilling_regs3.txt`  | 2 | spilling | 3 | `ranges7.txt` — K3 triangle, needs 1 spill with 2 registers (budget has slack)          |
| `splitting_regs1.txt` | 1 | splitting | 2 | `ranges5.txt  — needs 1 register               |
 | `splitting_regs2txt` | 1 | splitting | 2 | `ranges7.txt` — needs 1 split only             |

---

## Usage (temporary)

```
./cmake-build-debug/myProg ./Data/ranges/<ranges_file> ./Data/registers/<registers_file> ./Data/output/<output_folder>/<output_file>
```

Example:

```
./cmake-build-debug/myProg ./Data/ranges/ranges1.txt ./Data/registers/spilling_regs1.txt Data/output/spilling/out.txt
```