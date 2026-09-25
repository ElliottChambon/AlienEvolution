# AlienEvolution Development Roadmap

## Purpose

This roadmap connects the current prototype to the mature scientific
architecture described in `MATURE_ARCHITECTURE.md`.

Version numbers describe major capability milestones rather than promises of
fixed release dates.

The central development rule is:

> Early versions may simplify scientific mechanisms in order to validate the
> architecture, but those simplifications must remain explicit, replaceable,
> documented, and connected to the intended mature model.

AlienEvolution should grow toward the mature architecture rather than
accumulating disconnected prototype features.

---

# V0.1 — Complete Evolutionary Causal Loop

## Objective

Demonstrate that AlienEvolution can perform genuine evolutionary change
without directly encoding final traits.

The required causal chain is:

Heritable variation
→ developmental variation
→ phenotypic variation
→ physical/resource consequences
→ differential reproductive success
→ inheritance and mutation
→ population change.

## Development representation

V0.1 uses the existing simplified two-dimensional developmental material
field.

This representation is explicitly temporary and exists to validate the
genotype-development-phenotype-evolution architecture.

## Required systems

- Environment
- Genome / heritable parameters
- Development
- Phenotype
- deterministic random-number system
- Organism
- Population
- mutation
- reproduction
- selection
- fitness / consequence evaluation
- generation loop
- reproducible random seeds
- experiment configuration
- generation statistics
- automated tests

## Scientific requirement

Fitness must arise through consequences of phenotype and environment.

The environment must not directly assign traits.

The genome must not directly encode final organism dimensions or named organs.

## V0.1 completion criterion

A population subjected to mutation and selection must evolve measurable
changes in phenotype across generations.

Repeated runs with different random seeds must be capable of producing
different evolutionary histories.

Identical runs with the same seed and configuration must be reproducible.

---

# V0.2 — Evolvable Regulatory Development

## Objective

Replace the extremely small fixed genome with a more expressive regulatory
representation.

The purpose is to expand evolvable developmental space without directly
encoding anatomy.

## Candidate capabilities

- regulatory-network-like hereditary representation
- nonlinear gene/regulatory interactions
- conditional developmental behavior
- developmental memory
- polarity
- positional information
- symmetry breaking
- developmental timing
- multiple material states
- mutation of network structure as well as parameter values

## Scientific work

The exact regulatory representation must be selected through literature
review and benchmark testing.

V0.2 should begin moving from:

    seven independent parameters

toward:

    evolvable developmental control system.

---

# V0.3 — Three-Dimensional Developmental Units

## Objective

Move from the 2D material-field prototype to the first version of the mature
3D developmental architecture.

## Core additions

- DevelopmentalUnit abstraction
- off-lattice 3D positions
- orientation and polarity
- generalized internal state
- material identity
- developmental lineage
- local contact behavior
- local signaling
- continuous environmental/developmental fields
- three-dimensional phenotype state

## Scientific principle

DevelopmentalUnit does not automatically mean Earth biological cell.

Earth-like models may configure units to behave like cells, but the software
architecture should not require terrestrial cellular biology as a universal
assumption.

## Completion criterion

The simulator can develop reproducible three-dimensional multicellular or
multi-unit structures without specifying their final geometry.

---

# V0.4 — Materials, Mechanics, and Transport

## Objective

Allow morphology to acquire physical meaning.

Development should interact with mechanics and transport rather than producing
geometry that is evaluated only visually.

## Candidate systems

- differentiated material properties
- density
- elasticity
- viscosity
- anisotropy
- failure
- active contractility
- diffusion
- resource transport
- waste transport
- thermal transport
- internal flow
- body forces such as gravity
- stress and strain
- mechanically sensitive development
- transport-demand-sensitive development

## Solver research

Candidate methods include:

- off-lattice mechanics
- finite-element methods
- Material Point Method
- phase-field methods
- deformable-unit methods
- reduced beam/rod models
- shell/membrane mechanics
- network flow models

No mature solver should be selected solely because it is convenient to
implement.

## Completion criterion

Different physical functions can drive genuinely different geometry and
material organization.

An elongated transport structure and an elongated load-bearing structure
should be capable of diverging because they face different physical demands,
not because the simulator labels one "intestine" and the other "skeleton."

---

# V0.5 — Functional Organism Physics

## Objective

Evaluate organisms according to what their structures physically accomplish.

## Candidate functional measurements

- structural support
- force production
- locomotor work
- transport capacity
- exchange efficiency
- resource acquisition
- thermal control
- sensing
- information transfer
- protection
- reproduction

## Scientific principle

Function should primarily be inferred from measurable contribution.

The simulator should not require Earth-organ labels before physical function
can be evaluated.

## Example

A region might be internally described as:

    anisotropic stiff branched material
    carrying 47% of axial body load

The user interface may later describe it as:

    functionally analogous to a skeletal support structure.

---

# V0.6 — Spatial Ecology and Planetary Evolution

## Objective

Move evolution from isolated populations into planetary space and ecology.

## Candidate systems

- geographic environment maps
- spatial populations
- dispersal
- migration
- ecological niches
- competition
- predation or resource interactions
- symbiosis
- environmental change
- lineage divergence
- extinction
- speciation-like lineage splitting
- ecological succession

## Data model

Evolution should produce a lineage graph through time.

Each lineage may possess:

- ancestry
- geographic range
- population abundance
- phenotype summaries
- ecological relationships
- representative organisms

## Completion criterion

The simulation can generate a changing planetary map of biological lineages
through evolutionary time.

---

# V0.7 — Adaptive Physics and Multi-Fidelity Simulation

## Objective

Make large evolutionary experiments computationally practical without allowing
simplification to corrupt scientific conclusions.

## Core strategy

Use the least expensive physical representation that remains valid.

Possible transitions include:

3D bulk structure
→ beam / rod

thin material region
→ shell / membrane

branched conduit
→ reduced transport network

dense approximately homogeneous material
→ continuum representation

complex growth front
→ high-resolution local model

## Requirements

Every simplification must define:

- validity conditions,
- error tolerances,
- conversion rules,
- conserved state,
- promotion criteria,
- reduction criteria.

If assumptions fail, the system must automatically return the region to a
more general representation.

## Critical scientific requirement

Evolution must not be allowed to exploit approximation artifacts.

Low- and high-fidelity models must therefore overlap on benchmark problems.

Cross-fidelity evolutionary ordering must remain consistent within validated
regimes.

---

# V0.8 — Organism Archive and On-Demand Reconstruction

## Objective

Separate expensive high-resolution organism inspection from large evolutionary
simulation.

## Representative organism archive

A lineage record may preserve:

- heritable program
- random seed
- parent lineage
- model version
- environment
- developmental configuration
- material topology
- phenotype summary
- functional measurements
- geographic range
- population statistics
- intermediate-resolution developmental state

## Reconstruction

When a user requests an organism:

archive state
→ replay or refine development
→ reconstruct higher-resolution phenotype
→ derive visualization geometry
→ create functional annotations
→ cache result.

The planetary simulation therefore does not need to permanently store a
high-resolution mesh for every historical organism.

---

# V0.9 — Interactive Planetary Biology Explorer

## Objective

Create the public-facing AlienEvolution experience.

## Long-term interface

Users should be able to:

- view the planet,
- move through evolutionary/geological time,
- observe lineage ranges on a world map,
- inspect radiation, migration, and extinction,
- select a lineage,
- reconstruct a representative organism,
- rotate and inspect a 3D model,
- use cutaways,
- highlight internal structures,
- view physical and biological function,
- compare relatives,
- inspect ecology and environmental adaptations.

The experience should resemble an interactive scientific field guide or
nature documentary.

Descriptions should be grounded in simulation measurements wherever possible.

---

# V1.0 — Research-Grade AlienEvolution Framework

## Objective

Reach a validated architecture capable of supporting serious computational
experiments concerning evolutionary predictability under planetary
constraints.

## Research capabilities

Examples include:

- repeated independent evolutionary histories under identical environments,
- systematic environmental parameter sweeps,
- convergence analysis,
- contingency analysis,
- phenotype-space analysis,
- functional adaptation statistics,
- lineage diversity analysis,
- environmental counterfactual experiments.

The framework should support questions such as:

    P(adaptation | environment)

and investigate which functional properties are strongly predictable from
environment and which depend strongly on historical contingency.

## Scientific output

AlienEvolution should clearly distinguish:

- established physical constraints,
- empirically supported biological mechanisms,
- reasonable extrapolation,
- computational approximation,
- speculation,
- uncertainty.

The system should not claim to predict the unique form of extraterrestrial
life.

Its purpose is to explore scientifically constrained distributions of
possible evolutionary outcomes.

---

# Continuous Work Across All Versions

The following activities continue throughout development:

- literature review,
- scientific assumption tracking,
- software testing,
- numerical verification,
- model validation,
- benchmark expansion,
- reproducibility testing,
- performance profiling,
- cross-fidelity comparison,
- scientific documentation.

The mature architecture may evolve when new evidence or better methods become
available.

Scientific rigor takes priority over preserving an obsolete architectural
decision.