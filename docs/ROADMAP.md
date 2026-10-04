# AlienEvolution Development Roadmap

## Purpose

The [mature-commitment registry](MATURE_COMMITMENTS.md) tracks approved
directions and deferred capabilities separately from prototype progress.
Milestone completion must be assessed against its recorded validation evidence.

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

The validated B4–B6 sensing work and the new M0–M7+ migration below remain
distinct from this deferred 3D capability milestone.

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

## M0–M7+ mature architecture migration

The former proposed **B7 structural sensory mutation** direction is superseded
by this staged migration under [approved issue #21](https://github.com/ElliottChambon/AlienEvolution/issues/21).
B7 is not implemented or authorized by M0. This sequence does not implement
chemistry, quantum physics, mechanics, materials, or structural evolution all at once.

### Retained validated milestones

- **V0.3B4:** HeritableProgram composition and separate SensoryProgram ownership/
  inheritance, validated runtime adapter, preserved regulatory compatibility.
- **V0.3B5:** Explicit quantitative sensory mutation null model; fixed IDs and
  structure, no automatic target repair, zero-hazard RNG compatibility.
- **V0.3B6:** [Controlled sensory-selection validation](V0_3B6_VALIDATION.md):
  fixed regulation/structure, half-saturation evolution, matched selected/neutral/
  no-mutation lines and direct assays. Its evidence and limitations remain intact;
  it does not validate mature biology or authorize structural sensory mutation.

### M0 — Architecture freeze/documentation

Issue #21 records approved S1–S5 in the foundation documents and guardrails.
Documentation only; production behavior is unchanged. Freezing causal/interface
commitments does not complete their mature capabilities or select mechanisms.

### M1 — Scientific metadata/evidence/version primitives

Pure software metadata/interfaces only, with no production behavior change.
Record evidence categories, provenance, alternatives, versions/supersession,
units/parameters and separate scientific/model-form versus numerical uncertainty.

**Implemented/scaffolded (issue #23):**
[ScientificModelMetadata](../include/alien_evolution/science/ScientificModelMetadata.hpp)
provides standalone records and explicit syntactic validation, verified by
[fast metadata tests](../tests/scientific_metadata_tests.cpp). Evidence codes are
nominal categories, not a confidence ranking. Collections preserve insertion order;
supersession and alternatives are inert identifiers. Parameter units are declared
text (use `dimensionless` explicitly), with no conversion or dimensional analysis.
Assumptions and validity scope are human-readable declarations, not certification.
There is no scientific mechanism selection or biological validation, and no
production simulation integration. M2 is scaffolded below.

### M2 — Physical port + Physical Coupling Process software scaffold

Stable abstract data/interface layer for typed bidirectional ports, state/history,
control, accessible observables and exchange ledgers. No claim that current
sensing has migrated; no universal domain port catalog or solver is selected.

**Scaffolded (issue #25):**
[PhysicalCouplingProcessSchema](../include/alien_evolution/physics/PhysicalCouplingProcess.hpp)
provides schema vocabulary only: model-owned quantity/type identifiers with text
units, input/output/bidirectional port boundaries, separate ordered state, history,
control, observable and ledger descriptors, owned M1 scientific metadata, and an
optional inert region binding identifier. Local keys are unique within each
category; quantity keys inside ports are local to each port. Explicit validation
checks syntax only and allows empty categories. It selects no physical domain
mechanism, executes no PCP, and provides no biological validation. Matching type
identifiers or units do not establish connections or compatibility. Current
sensing remains unmigrated. M3 contracts are scaffolded below.

### M3 — Adaptive Certified Physics scaffold

Validity contracts, quantity-of-interest/error/provenance hooks, reference/reduced
model relationships and challenge-case registry concepts. Include selection-aware
promotion/audit boundaries; solver choice stays simulator-owned.

**Scaffolded (issue #27):**
[Adaptive Certified Physics contracts](../include/alien_evolution/physics/AdaptiveCertifiedPhysics.hpp)
record exact model ID/version references, M2 quantity descriptors as QoIs,
declared validity criteria, numerical/reduction error-measure declarations tied
to QoIs, reference-model edges, typed audit reasons, inert review requests, and
an append-only in-memory challenge registry with read-only lookup. Collections
preserve insertion order; reference edges apply to a contract's declared QoIs
and do not define a universal fidelity ladder or biological truth. Contracts
require at least one QoI; criteria, error measures and reference models may be
absent. Review/challenge QoI lists may be empty; supplied keys must be nonblank,
and optional contract-scoped syntax checks require declared-key membership.
Challenge context/reproduction references are stored as text, not loaded or
verified; registry retention is for its in-memory lifetime, with no persistence.
No validity predicate or error estimator is executed, no solver/model selection
occurs, and no scientific mechanism is certified. Challenge cases are simulator
validation artifacts, never inherited organism state. Selection-aware audit
reasons can record concerns that approximation could alter a scientific or
evolutionary decision; policy thresholds remain deferred. Pruning must never
erase future evolutionary possibilities. Production behavior and current sensing
remain unchanged. M4 Heritable Construction State compatibility scaffold is next.

### M4 — Heritable Construction State compatibility scaffold

Introduce the mature wrapper/abstraction while preserving current HeritableProgram
through an adapter/migration path. Do not universalize DNA or directly evolvable
finished material/sensory properties.

### M5 — Provenance DAG + causal dependency/invalidation infrastructure

Separate immutable multi-parent history from derived causal dependencies.
Both are simulator-owned, non-inherited, with no physical influence on organisms;
dependencies support pleiotropy-aware invalidation and recomputation.

### M6 — First migrated coupling benchmark

Re-express one deliberately simple current phenomenon through PCPs. Verify
equivalence to the old validated mechanism in its declared regime before replacing
anything. Preserve deterministic compatibility and B4–B6 history/evidence;
equivalence is not mature biological validation.

### M7+ — Gradual domain mechanisms and structural evolution

Proceed only after explicit scientific architecture checkpoints for the specific
mechanism/inheritance backend, with literature review, evidence/alternatives,
validity/version records, and benchmarks. Physical inheritance opportunities and
mutation footprints replace arbitrary graph edits; novelty and function are
diagnosed after development/physics. Add neutral accessibility audits and
evolutionary adversarial validation as the relevant capabilities land.

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
