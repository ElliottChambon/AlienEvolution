# Scientific Model Debt

AlienEvolution uses simplified models during early development to validate
the architecture and causal behavior of the simulation.

These simplifications are not intended to define the final scientific model.

Where feasible, simplified mechanisms should be replaced or extended as the
project matures with models that better reflect established scientific
knowledge.

A simplification is acceptable when it helps validate architecture,
algorithms, or causal behavior, but it should not silently become a permanent
scientific assumption.

The long-term objective is to progressively move AlienEvolution toward the
highest scientifically defensible fidelity that is practical for the research
question being studied.

---

# V0.1 Development and Phenotype

## Two-Dimensional Phenotype Field

### Current model

Organismal morphology is represented as a two-dimensional scalar material
field.

Each grid location stores an occupancy value between:

    0.0 = no organismal material
    1.0 = fully occupied material

### Why this exists

A 2D material field allows the complete:

heritable information
→ development
→ phenotype
→ physical consequence
→ evolution

pipeline to be developed and tested before introducing substantially more
expensive three-dimensional developmental mechanics.

It also provides a simple representation that can be visualized and debugged
during early implementation.

### Limitations

The representation does not capture:

- true three-dimensional geometry,
- tissue or material differentiation,
- internal anatomy,
- cavities,
- complex topology,
- cellular organization,
- realistic developmental signaling,
- mechanical anisotropy,
- realistic embryological development,
- transport networks,
- internal fluid systems,
- active materials,
- physiologically meaningful organs.

The 2D field should therefore not be interpreted as a realistic organism
representation.

### Future direction

Replace or extend the field with a three-dimensional developmental
representation incorporating physically and biologically meaningful
mechanisms.

Candidate mature approaches should be evaluated against contemporary research
in:

- developmental biology,
- evolutionary developmental biology,
- biomechanics,
- morphogenesis,
- biological materials,
- transport systems,
- multiscale modeling.

The choice should be driven by the scientific questions AlienEvolution is
intended to answer rather than visual complexity alone.

---

# Local Developmental Signaling

## Current model

V0.1 restricts developmental signaling to the eight neighboring locations of
a two-dimensional grid.

Signal strength decays exponentially with distance.

Growth requires a positive regulatory signal generated from existing
organismal material.

Environmental resource availability can modulate growth but cannot
independently create new organismal material.

### Why this exists

This local rule provides a computationally inexpensive mechanism for testing
whether heritable developmental parameters can generate phenotypic variation
without directly encoding final morphology.

It also ensures that development expands from existing organismal material
rather than allowing biological matter to appear spontaneously throughout the
simulation domain.

### Limitations

The current model is not intended to represent a specific known developmental
signaling pathway.

It currently does not model:

- cells or generalized developmental units,
- gene-regulatory networks,
- morphogens,
- diffusion,
- reaction kinetics,
- tissue mechanics,
- cell division,
- material differentiation,
- polarity,
- developmental timing,
- long-range signaling,
- mechanochemical feedback,
- three-dimensional morphogenesis.

The local 3x3 neighborhood and exponential weighting are computational
modeling choices rather than claims about universal biological development.

### Future direction

Replace or extend the local signaling abstraction with mechanisms supported
by developmental and physical science.

Candidate mature mechanisms may include:

- gene-regulatory-network-like systems,
- reaction-diffusion systems,
- developmental-unit signaling,
- morphogen transport,
- mechanochemical feedback,
- polarity and positional information,
- tissue mechanics,
- explicit three-dimensional development.

The mature architecture should remain capable of representing life that does
not necessarily use Earth-specific cellular or molecular mechanisms.

---

# Simplified Heritable Representation

## Current model

V0.1 currently represents hereditary developmental information using a small
set of numerical parameters such as:

- regulatory strength,
- environmental response strength,
- developmental threshold,
- spatial signaling scale,
- response sensitivity,
- growth rate,
- material or metabolic cost.

These parameters influence development rather than directly specifying final
traits.

### Why this exists

A small numerical genome is sufficient to test:

heritable variation
→ altered development
→ altered phenotype
→ altered consequences
→ selection.

### Limitations

The mature genotype-to-phenotype relationship is expected to be much more
complex.

The current representation cannot express:

- regulatory networks,
- modular developmental programs,
- developmental memory,
- lineage-dependent regulation,
- conditional gene expression,
- regulatory evolution,
- complex genotype interactions,
- developmental plasticity,
- large-scale evolvability.

### Future direction

Investigate more general heritable regulatory systems based on relevant
literature.

The mature representation should control developmental processes rather than
directly encode structures such as:

- legs,
- eyes,
- mouths,
- organs,
- skeletal elements.

Final anatomy should emerge from the interaction between heredity,
development, materials, physics, environment, and evolution.

---

# Fixed Developmental Representation

## Current model

All V0.1 development currently uses the same fixed two-dimensional grid
representation.

Every region is simulated with the same level of physical and developmental
detail.

### Limitation

A single fixed representation is unlikely to make mature AlienEvolution
computationally feasible.

Planet-scale evolutionary studies may eventually require extremely large
numbers of developmental evaluations.

Using maximum physical resolution everywhere would therefore make large
evolutionary experiments prohibitively expensive.

### Future direction

AlienEvolution should support adaptive multiscale representation.

Regions should be represented using the least expensive model that remains
scientifically valid for their current physical regime.

---

# Adaptive Fidelity and Reduced Physics

## Current status

AlienEvolution does not yet implement adaptive multiscale physics.

The current V0.1 developmental field uses a single fixed representation.

## Mature requirement

The mature engine is expected to simplify physical regions when established
reduced-order approximations are valid.

Potential examples include:

- beam or rod mechanics for sufficiently slender structures,
- shell or membrane mechanics for sufficiently thin surfaces,
- reduced flow-network models for appropriate transport structures,
- continuum coarse-graining for sufficiently homogeneous dense material,
- simplified transport or diffusion models where their assumptions hold.

Each simplification must define explicit mathematical validity conditions.

For example, a structure should only be converted to beam mechanics when its:

- geometry,
- material properties,
- loading,
- deformation,
- and relevant boundary conditions

fall within a regime where beam theory is sufficiently accurate.

If those assumptions cease to hold, the region should be promoted back to a
more general physical representation.

## Scientific risk

Evolutionary optimization can exploit numerical artifacts.

A phenotype that appears superior only because a reduced model is inaccurate
must not be allowed to dominate an evolutionary simulation.

This is particularly dangerous because evolution actively searches the space
of possible solutions and may discover edge cases that exploit approximation
errors.

Therefore adaptive simplification requires cross-fidelity validation against
more general models within overlapping regimes.

## Future direction

Develop quantitative:

- fidelity-transition criteria,
- error tolerances,
- state-conversion methods,
- conserved-quantity mappings,
- promotion criteria,
- reduction criteria,
- cross-fidelity benchmarks.

Adaptive simplification is expected to become a major scalability mechanism
for mature AlienEvolution.

---

# Developmental Units

## Current status

The V0.1 material field does not contain explicit developmental units.

### Mature requirement

The mature architecture currently proposes a generalized localized entity
called a:

    DevelopmentalUnit

rather than assuming that all extraterrestrial life must use Earth-like
cells.

A DevelopmentalUnit may eventually carry:

- position,
- orientation,
- polarity,
- regulatory state,
- material state,
- energetic state,
- developmental lineage,
- signal production,
- signal sensing,
- uptake and secretion behavior,
- adhesion or contact properties,
- active mechanical properties.

An Earth-like model may interpret these units as cells.

Other models may use different physically and chemically plausible
interpretations.

### Scientific uncertainty

The DevelopmentalUnit abstraction is an architectural generalization, not a
claim that extraterrestrial organisms must possess discrete cell-like units.

Its mature implementation requires continued review of:

- origins-of-life research,
- astrobiology,
- alternative biochemistry,
- developmental biology,
- theoretical biology.

---

# Material Differentiation

## Current status

V0.1 material has only one scalar occupancy state.

All occupied material is physically identical.

### Limitation

This prevents development of physically distinct structures.

A mature organism may require regions that differ in properties such as:

- density,
- stiffness,
- elasticity,
- viscosity,
- anisotropy,
- permeability,
- thermal properties,
- failure strength,
- chemical reactivity,
- electrical behavior,
- active contractility,
- resource storage.

Without material differentiation, structures with different physical roles
cannot properly diverge.

### Future direction

Add evolvable and developmentally regulated material states.

Material categories should be defined primarily through physical and chemical
properties rather than Earth-specific anatomical names.

---

# Mechanics

## Current status

V0.1 does not yet simulate realistic organismal mechanics.

Gravity is stored in the environment but is not currently allowed to directly
change morphology through arbitrary rules.

### Why this matters

AlienEvolution should not contain shortcuts such as:

    high gravity -> short organism

Instead, gravity should affect:

- body forces,
- stresses,
- structural support,
- locomotion,
- material requirements,
- energetic costs,
- failure probability.

Evolution can then respond to those physical consequences.

### Future direction

Investigate adaptive combinations of established mechanics methods including:

- off-lattice agent mechanics,
- continuum mechanics,
- finite-element methods,
- Material Point Method,
- phase-field methods,
- beam and rod models,
- shell models,
- deformable-unit methods.

The exact mature mechanics stack remains an open research decision.

---

# Transport and Internal Networks

## Current status

V0.1 does not model internal transport.

### Limitation

Transport constraints are likely to be essential for sufficiently large or
complex organisms.

Relevant quantities may include:

- nutrients,
- reaction substrates,
- waste,
- gases,
- heat,
- signaling molecules,
- electrical signals,
- internal fluids.

### Future direction

Allow transport structures to develop from local regulatory and physical
mechanisms.

Their geometry should be evaluated by actual transport performance rather
than visual resemblance to Earth vascular or digestive systems.

Reduced network representations may eventually be used when mathematically
valid.

---

# Function and Anatomy

## Current status

V0.1 does not yet contain functional anatomical classification.

### Mature requirement

The simulation should not require structures to be assigned labels such as:

    Leg
    Skeleton
    Intestine
    Eye
    BloodVessel

before their function can be evaluated.

Instead, function should arise from measurable physical contribution.

Examples include:

- load bearing,
- force production,
- locomotor work,
- material transport,
- exchange,
- heat transfer,
- energy capture,
- sensing,
- information transmission,
- defense,
- reproduction.

A user-facing interface may describe a structure as:

    functionally analogous to a skeletal support

or:

    a digestive/absorptive transport structure

when such analogies are useful.

The underlying simulator should retain the more general physical description.

---

# Visualization

## Current status

V0.1 uses a terminal representation of a two-dimensional phenotype.

### Mature requirement

Scientific phenotype state and visualization geometry must remain separate.

Long-term simulations should not need to permanently store a high-resolution
render mesh for every organism.

Instead, representative organism records should preserve enough scientific
state to reconstruct or refine an organism when requested.

Potential archive data include:

- heritable program,
- lineage identity,
- environmental state,
- deterministic seed,
- model version,
- material topology,
- functional measurements,
- developmental summaries,
- low- or intermediate-resolution phenotype state.

A high-resolution 3D model may then be reconstructed on demand and cached.

### Scientific requirement

Visualization must not silently invent biologically meaningful structures that
were absent from the simulation.

Any reconstructed detail that exceeds simulation fidelity should be treated as
visual reconstruction rather than established simulation output.

---

# Cross-Fidelity Consistency

## Current status

V0.1 currently uses only one developmental fidelity level.

### Mature requirement

Low-, intermediate-, and high-fidelity representations should overlap on
benchmark problems.

Important evolutionary conclusions should remain compatible across fidelity
levels within validated regimes.

If a low-fidelity model predicts that phenotype A outperforms phenotype B but
a validated higher-fidelity model reverses that conclusion, the low-fidelity
model is not scientifically adequate in that regime.

Cross-fidelity testing is therefore part of the scientific architecture, not
merely a performance optimization.

---

# Guiding Rule

AlienEvolution may simplify aggressively during development and large-scale
simulation, but simplification must always be:

1. explicit,
2. documented,
3. scientifically motivated,
4. associated with known limitations,
5. replaceable,
6. tested against higher-fidelity models when possible.

The objective is not to make the simplest possible alien generator.

The objective is to create a computational framework whose models can become
progressively more rigorous as scientific knowledge, computing resources, and
the project itself mature.