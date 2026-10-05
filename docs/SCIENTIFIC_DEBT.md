# Scientific Model Debt

See the [mature-commitment registry](MATURE_COMMITMENTS.md) for the master audit
of frozen directions, required capabilities with open implementations, and
prototype debt. Update both records when introducing or replacing a simplification;
passing prototype tests does not close a mature commitment.

## S1–S5 replacement map (M0, issue #21)

The mature replacements are approved architecture, not implemented mechanisms.
`SensoryProgram` is the validated B4–B6 prototype compatibility/scaffold; it is
not the mature sensing endpoint. [Sensing foundations](SENSING_FOUNDATIONS.md),
[PCP architecture](PHYSICAL_COUPLING_ARCHITECTURE.md), and
[Heritable Construction](HERITABLE_CONSTRUCTION_ARCHITECTURE.md) define the target.

| Current simplification | Mature replacement / remaining work |
| --- | --- |
| Arbitrary signal/output/target IDs; channel index as event identity | Physical access, recognition/compatibility/localization/timing/regulation through PCP networks; separate ancestry, current construction, and current role; no ID-retarget mutations |
| Nonnegative scalar inputs, shifted-Hill response, independent multiplicative channels, constant sampled inputs | Typed bidirectional physical ports, state/history, observables and exchange ledgers; domain-specific models with noise, dynamics and uncertainty where justified; channels only certified reductions |
| Direct fold-change/half-saturation/cooperativity mutation (B5); half-saturation only in B6 | G mutation through inheritance-dependent events -> coupled development -> materials/geometry -> compiled responses -> PCPs; direct effective-parameter mutation remains a scoped reduced experiment |
| Graph gain/loss/duplication and software opportunity counts | Physical mutation footprints and inheritance opportunity hazards; no silent edge cloning, automatic deletion/retargeting, or interface repair; concrete backend still open |
| Seven-parameter history / current RegulatoryProgram + SensoryProgram ownership | Chemistry-agnostic Heritable Construction State with an adapter for current HeritableProgram; inheritance does not universally require DNA |
| 2D deposition and scalar occupancy, local neighborhood growth | Coupled development with material composition/phase/microstructure/orientation/interfaces/state/history and evolving geometry/topology; 3D, mechanics and transport still deferred |
| Boundary-based energy, no modeled sensing costs/feedback | Actual physical/resource exchanges for construction, maintenance, repair, amplification, emission and nonequilibrium operation; no universal sensorCost |
| Fixed numerical representation, no certification/adversarial registry | Quantity-specific Adaptive Certified Physics, reference/reduced relationships, selection-aware promotion and permanent challenge cases; evolution cannot mutate solver choice |
| Limited component event records; no mature historical/dependency infrastructure | Immutable multi-parent provenance DAG separate from derived causal dependencies and pleiotropy-aware invalidation; neither physically influences organisms or is inherited |

M0 does not retire any implementation. [M1–M7+](ROADMAP.md#m0m7-mature-architecture-migration)
stages migration, with first-coupling equivalence required before replacement.
The [B6 fixture and measured results](V0_3B6_VALIDATION.md) remain controlled
prototype mechanism evidence. No mature sensing, material, inheritance, or
biological validation is claimed. Layer-specific evidence categories and model
versions/alternatives must accompany future replacements; scientific/model-form
uncertainty remains separate from numerical/reduction uncertainty.

## Heritable Construction State compatibility scaffold (M4, issue #29)

[HeritableConstructionState](../include/alien_evolution/genetics/HeritableConstructionState.hpp)
now provides the migration seam, owning only the current `HeritableProgram`
prototype heredity payload. `Organism` owns HCS without a duplicate program;
named prototype adapters and legacy accessors preserve current behavior.
Mutation/reproduction algorithms, random consumption and B4–B6 behavior remain
unchanged. This is inherited organismal state, not scientific metadata or
simulator-owned provenance. No mature inheritance chemistry is selected and no
structural novelty, provenance or dependency mechanism is implemented. Existing
direct parameter mutation and graph opportunity debt remain open; the wrapper
does not reinterpret B6 evidence as mature construction-state mutation or provide
biological validation. M5 bookkeeping is scaffolded below; mechanism-specific
inheritance still requires review.

## Historical and dependency bookkeeping scaffold (M5, issue #31)

[EvolutionaryProvenanceGraph](../include/alien_evolution/evolution/EvolutionaryProvenance.hpp)
records simulator-owned immutable history; the separate
[CausalDependencyGraph](../include/alien_evolution/core/CausalDependencyGraph.hpp)
records simulator-owned recomputation dependencies. Neither graph is inherited state.
Roots use parentless de novo records; existing parents and fresh children prevent
ancestry cycles. Dependency cycles are allowed and closure queries terminate.
Affected closure includes changed roots and downstream artifacts in registration
order, with no actual incremental recomputation, cache mutation or biological effect.
These are standalone in-memory scaffolds, not production simulation integrations.
Current prototype regulatory/sensory IDs have not been assigned mature historical
identities. No structural mutation or mutation footprint model exists yet; chemistry,
identity integration and inheritance-event semantics still require scientific review.
No biological validation is claimed. M6 prototype PCC migration is benchmarked below; M7+ domain mechanisms still require review.

## M6 prototype shifted-Hill PCC migration benchmark

[PrototypeShiftedHillCouplingChannel](../include/alien_evolution/physics/PrototypeShiftedHillCouplingChannel.hpp)
is a deterministic memoryless phenomenological **reduced PCC**, not a full PCP.
The only claim is that it reproduces the validated V0.3 single-channel external
response mathematics within its declared scalar regime. It calls the existing
stable shifted-Hill kernel and has fixed finite positive parameters and a finite
nonnegative input in caller/model-defined units. Model ID/version is
`alien_evolution.prototype.shifted_hill_pcc` / `m6-v1`. M1 metadata names source
code and V0.3 prototype debt as provenance; evidence records remain empty rather
than assigning a stronger scientific category or B1 biological evidence.

M2 schema declares one neutral input and one dimensionless modulation observable;
state/history/control/ledger are absent. M3 metadata declares the scalar assumptions
and numerical compatibility measure/reference only. No validity predicate, formal
physical error estimator or solver promotion is executed. Zero bitwise migration
difference does not reduce scientific/model-form uncertainty. Mathematical output
is positive, but extreme finite repression can round the legacy result to zero;
M6 preserves that numerical limitation instead of changing production mathematics.
Transport/access, receptor chemistry, physical noise, energy/material exchanges,
adaptation and material coupling remain absent. No fake sensing cost is introduced.

The mature sensing target remains:

```text
world/environment physical state
-> propagation / physical accessibility
-> organism-developed material + geometry
-> physical coupling process(es)
-> local material/molecular/field state change
-> stochastic and/or deterministic transduction
-> amplification / filtering / control where evolved
-> regulatory / physiological integration
-> behavior / development / ecology / reproduction
```

Future reviewed mechanisms may need transport/access, binding/association, chemical
transformation, force/strain transfer, charge transport/polarization, photon
absorption/excitation, magnetic/spin dynamics, or ionization/energy deposition.
Geometry may filter/focus/resonate; composition and microstructure, physical noise
and detection limits, nonequilibrium costs, bidirectional exchange, adaptation/
hysteresis and stochastic/hybrid dynamics must be represented where relevant.
No universal receptor mechanism is selected; domain models remain literature-reviewed,
versioned and explicit about evidence, alternatives and scientific uncertainty.

Effective gain, affinity, half-saturation, spectral response, stiffness, conductivity
and similar parameters should generally derive from developed phenotype/material
physics or be explicitly certified reduced parameters. Evolution changes HCS,
development, material and geometry, never the scientific solver. Sensing emerges
from PCP networks, not a permanent `SensoryProgram` channel catalog. A future PCC
may serve as a compiled reduction only within its certified validity region;
phenotype/material changes outside it must eventually trigger invalidation,
promotion/refinement against more general physical models. Full exchange/conservation
accounting, physical noise, energetic costs and model-form uncertainty remain
required where relevant. None of that future machinery is implemented by M6.

The replacement path is explicit:

```text
CURRENT: SensoryProgram -> RegulatoryInputChannel -> shifted-Hill modulation
M6: same mathematics as a prototype PCC with scientific/schema/validity metadata
FUTURE M7+: physical mechanism PCP chain -> certified compiled PCC where justified
            -> internal regulatory integration
EVENTUAL: retire the current SensoryProgram abstraction only after validated
          domain mechanisms and reviewed migration exist
```

Production `Simulation` still uses `RegulatoryInputInterface`; all B4–B6 behavior
and evidence remain unchanged. M6 schedules no deletion, supplies no universal PCP
runtime API and migrates no multiple-channel multiplication. M7+ requires explicit
mechanism checkpoints and literature/benchmark evidence before production replacement.

## External signal transduction interface (V0.3B1)

`RegulatoryInputInterface` is an isolated phenomenological external-signal/
transduction abstraction, not a claim of universal Earth-like receptor biology.
It maps caller-supplied nonnegative signal magnitudes to multiplicative
regulatory target modulation using the existing shifted-Hill response. Signal
units and normalization remain the caller's responsibility; half-saturation
must use the same units as its signal.

Independent channels multiply their responses. This simplifies transduction
and omits sensor dynamics, adaptation, cross-channel coupling, sensing costs,
and explicit chemistry. Physical quantities that can be signed require an
explicit upstream conversion to a nonnegative signal; no such conversion is
implemented here. V0.3B2 connects the interface to regulatory dynamics by
multiplying production at every RK4 derivative stage. Supplied signals remain
constant throughout a step (or a constant-input simulation), with no modeled
sensor kinetics or feedback to the external field. V0.3B3 adds an opt-in
development overload supplying the existing local-material measurement and
nonnegative resource availability as external signals. Measurements are sampled
from the current phenotype and held constant during each location's regulatory
substeps; phenotype and regulatory states still update synchronously. The legacy
clamp path remains the production simulation path, so current organism behavior
is unchanged. The diffusive field is not connected.

The [historical approved sensory-program separation](MATURE_COMMITMENTS.md#6-separate-heritable-sensoryprogram)
provided a separate, jointly inherited prototype component alongside internal
regulation. V0.3B4 implements `HeritableProgram` ownership of separate
`RegulatoryProgram` and `SensoryProgram` components. The sensory container uses
the existing ID-based shifted-Hill channels as a temporary phenomenological
representation, not a mature receptor model. It derives the validated runtime
adapter without duplicating transduction logic. The compatibility reproduction
path copies sensory data unchanged and retains the regulatory mutation/RNG sequence.

Regulatory topology changes may leave inherited sensory targets unusable. B4
preserves those channels without deletion, retargeting, or repair; constructing
the runtime adapter rejects invalid targets. Co-evolution semantics are not
decided here. Structural sensory evolution remains a separate user-reviewed
architecture checkpoint. This step provides architecture/software and numerical
validation, not biological validation.

Before selecting a mature sensing mechanism, external literature review and
quantitative benchmarks are still required for response laws, channel
integration, timescales, and energetic costs in the modeled physical/chemical
setting. The current response is a replaceable architectural assumption.

## Quantitative sensory mutation null model (V0.3B5)

`SensoryMutation` applies `p' = p * exp(epsilon)` with
`epsilon ~ Normal(0, sigma)` to existing channels' fold change, half-saturation,
and cooperativity. Rates (expected events per channel per replication) and effect
scales are explicit model inputs, not biological constants; all default to zero.
Waiting distances follow a constant-hazard Poisson process and channel selection
is uniform, both deliberate null-model assumptions. The event safety limit throws
rather than silently truncating mutation. Invalid multipliers and parameter
overflow/underflow are rejected, without clipping or repairing the model.

`HeritableMutationConfig` keeps regulatory and sensory configuration separate.
Generation runs regulation first, then sensing, and returns both component event
records. Zero sensory hazard draws no random numbers, preserving historical
regulatory reproduction results and RNG state. The opt-in heritable reproduction
overload places configuration before RNG to avoid ambiguity with legacy calls
using an empty braced regulatory configuration. Production simulation retains
the regulatory-only compatibility overload and legacy clamp development.

Signal IDs, target node IDs, channel count, and ordering remain fixed. Channel
index is temporary event identity and must be reviewed before structural sensory
evolution. Regulatory node loss can still leave unusable sensory targets; runtime
adapter validation rejects these without deletion, retargeting, or co-mutation.
No new sensing modalities, costs, noise, delay, or adaptation are modeled.

This is architecture/mechanism verification, not biological validation. The
[V0.3B6 controlled experiment](V0_3B6_VALIDATION.md) now supports directional
selection on half-saturation relative to matched neutral lines in one declared
resource environment. Those experiments were not run in B5. Structural sensory evolution remains behind
an explicit user-reviewed architecture checkpoint. The
[mature sensing commitment](MATURE_COMMITMENTS.md#6-separate-heritable-sensoryprogram)
remains scaffolded: specificity must ultimately follow physical/chemical coupling,
not mutations between arbitrary integer signal IDs.

## Controlled sensory-selection benchmark limits (V0.3B6)

B6 freezes regulation, channel structure, fold change, and cooperativity and
evolves only half-saturation with the B5 null model. The deterministic landscape
selects resource 2 and a lower-K prediction before any stochastic run. The full
30-replicate result supports that prediction, with paired adaptive shift 0.481431
and approximate 95% mean interval [0.339411, 0.623452]; five of 30 paired shifts
were negative. Direct response assays confirm the ensemble sensory change.

All candidate gradients have the same sign and sampled optima lie on the lower
scan boundary. Environment-specific reversal, reciprocal adaptation, and a global
optimum are not established. Selected lines did not converge to that boundary
within 100 generations. Fixed-size populations, one scalar resource, arbitrary
signal/output IDs, shifted-Hill sensing, 2D deposition, and boundary-based energy
remain prototype constraints. Seeds are matched, but divergent parent selection
does not guarantee identical later RNG consumption. Descriptive intervals are
not calibrated coverage guarantees. No costs, noise, delay, receptor dynamics,
empirical calibration, or mature physics were added or validated.

The [actual results and frozen fixture](V0_3B6_VALIDATION.md) are mechanism evidence,
not mature biological validation. Production simulation retains its legacy path.
No parameters were tuned after full outcomes and no architecture commitment was
changed. Structural sensory evolution still requires explicit user review.

---

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

# CHEM-1A analytical and well-mixed scaffold

[Issue #35's approved architecture](CHEMICAL_COUPLING_ARCHITECTURE.md) is executable
for ideal dilute spherical rates, single-site noncooperative equilibrium, exact
constant-reservoir deterministic relaxation and well-mixed two-state CTMC only.
Effective rates, rebinding and `K_D` are compiled benchmark quantities with no
mutation path. P1 physical interaction evidence is not biological sensory-use
evidence; numerical checks do not certify scientific/model-form adequacy.

The spatial stochastic reference remains unselected/unimplemented. Isolated-pair
Green's-function, Brownian/Smoldyn-style, GFRD/eGFRD, RDME and general 3D PDE/
many-particle methods need a separate scientific checkpoint before backend selection.
The inert `review-pending` reference and eight UNRESOLVED challenge cases are not
spatial validation. Rebinding/arrival history, finite-domain depletion/conservative
matter exchange, geometry variation and selection-aware adversarial comparisons
remain untested physically.

Concentration is an ideal-dilute reservoir input, not universal activity. Nonideal
thermodynamics, mixtures/competition, cooperativity, electrochemistry/redox/protonation,
explicit driving, amplification, adaptation and proofreading remain deferred
mechanisms requiring review. No general chemistry engine, support framework,
runtime PCP wiring or automatic ACP policy is added. Probability balance does not
implement mature bidirectional matter/energy accounting. Finite-double range limits
and the nonunique equilibrium at `c=k_d=0` are explicitly documented.

Production sensing/evolution and M6 are unchanged. The mature target is materially
caused conservative coupling with quantity-specific validation, not a permanent
one-way occupancy-to-trait rule. M7A completes the scaffold for later cross-validation.

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
