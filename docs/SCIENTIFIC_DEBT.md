# Scientific Model Debt

AlienEvolution uses simplified models during early development to validate
the architecture and causal behavior of the simulation.

These simplifications are not intended to define the final scientific model.
Where feasible, they should be replaced or extended as the project matures
with models that better reflect established scientific knowledge.

## V0.1 Development and Phenotype

### Two-dimensional phenotype field

### Local developmental signaling

**Current model**

V0.1 restricts developmental signaling to the eight neighboring cells of a
two-dimensional grid. Signal strength decays exponentially with distance.

Growth requires a positive regulatory signal generated from existing
organismal material. Environmental resource availability modulates growth
but cannot independently nucleate new organismal material.

**Why this exists**

The local rule provides a computationally inexpensive mechanism for testing
whether heritable developmental parameters can generate phenotypic variation
without directly encoding final morphology.

**Limitations**

The model is not intended to represent a specific known developmental
signaling pathway. It does not currently model cells, gene-regulatory
networks, morphogens, diffusion, reaction kinetics, tissue mechanics,
cell division, differentiation, polarity, developmental timing, or
three-dimensional morphogenesis.

The 3x3 neighborhood and exponential weighting are computational modeling
choices rather than claims about universal biological development.

**Future direction**

Replace or extend this abstraction using developmental mechanisms supported
by contemporary developmental biology and evolutionary developmental
biology. Candidate models may include gene-regulatory networks,
reaction-diffusion systems, cell-based development, morphogen transport,
mechanochemical feedback, tissue mechanics, and explicit three-dimensional
growth. Model choice should depend on the scientific question being tested.

**Future direction**

Investigate replacement or extension with a three-dimensional developmental
representation incorporating physically and biologically meaningful
mechanisms. Candidate approaches should be evaluated against contemporary
knowledge in developmental biology, biomechanics, morphogenesis, and
evolutionary developmental biology rather than selected solely for visual
complexity.