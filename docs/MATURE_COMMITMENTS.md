# Mature-model commitments

## Purpose and authority

This registry records approved directions, required capabilities, and prototype
simplifications so architecture tests do not accidentally become the final model.
It implements [issue #12](https://github.com/ElliottChambon/AlienEvolution/issues/12)
and audits the current repository against [the mature architecture](MATURE_ARCHITECTURE.md),
[scientific debt](SCIENTIFIC_DEBT.md), [roadmap](ROADMAP.md), and
[benchmarks](BENCHMARKS.md). Conceptual commitments supplied by issue #12 are
recorded here; they are not newly selected scientific mechanisms or solvers.
The sensory separation specifically records the approved
[architecture decision in issue #11](https://github.com/ElliottChambon/AlienEvolution/issues/11).

## Status and maturity are separate

- **Frozen architectural commitment**: an explicitly approved direction; do not
  silently change it. A frozen direction does not freeze its numerical method.

- **Required mature capability, implementation open**: the capability is required;
  mechanisms and methods still need literature review and benchmarks.

- **Prototype scientific/numerical debt**: a deliberately simplified mechanism,
  including architectural/statistical debt, that is not the mature model.

Entry 26 is optional tooling outside these three scientific commitment categories.
It is not a required mature capability.

Each entry has a separate **Maturity** audit. `Deferred` means no mature
implementation exists; `Scaffolded` means only a prototype or partial boundary
exists. `Replaced and validated` requires an identified replacement, its supported
regime, and linked validation/benchmark evidence. No mature commitment below is
currently claimed as fully replaced and validated. Passing unit tests or CI is
software/numerical verification, not empirical biological validation.

This is an implementation snapshot, not a release schedule. Earlier V0.1 sections
in existing documents describe historical prototype debt; the active code uses
`RegulatoryProgram`. Keep progress and evidence current as replacements land.

## Audit evidence

These sources locate the current scaffolds; they do not establish mature-model
or biological validation. Use [BENCHMARKS.md](BENCHMARKS.md) to plan replacement
evidence and retain the distinction from the historical
[V0.1 architecture validation](V0_1_VALIDATION.md).

| Registry areas | Current implementation / verification anchors |
| --- | --- |
| Planetary inputs and population history (1–4, 25) | [Environment](../include/alien_evolution/environment/Environment.hpp), [Simulation](../src/simulation/Simulation.cpp), [controlled selection experiment](../experiments/regulatory_selection_validation.cpp) |
| Heredity, sensory boundaries, regulation (5–7, 11) | [RegulatoryProgram](../include/alien_evolution/genetics/RegulatoryProgram.hpp), [RegulatoryInput](../include/alien_evolution/genetics/RegulatoryInput.hpp), [dynamics tests](../tests/regulatory_dynamics_tests.cpp) |
| Development, material, geometry (8–11, 13, 20) | [RegulatoryDevelopment](../src/development/RegulatoryDevelopment.cpp), [Phenotype](../include/alien_evolution/development/Phenotype.hpp), [development tests](../tests/regulatory_development_tests.cpp) |
| Field physics (12) | [DiffusiveField2D](../src/physics/DiffusiveField2D.cpp), [field tests](../tests/diffusive_field_tests.cpp) |
| Energetics, population mechanics, mutation, function (16–19) | [Energetics](../src/evaluation/Energetics.cpp), [Fitness](../src/evaluation/Fitness.cpp), [Reproduction](../src/evolution/Reproduction.cpp), [mutation opportunities](../src/genetics/RegulatoryMutationRateModel.cpp), [mutation targets](../src/genetics/RegulatoryMutationTargetSelector.cpp), [PhenotypeMetrics](../src/evaluation/PhenotypeMetrics.cpp) |
| Archive scaffold and software verification (21, 23–24) | [Organism](../include/alien_evolution/evolution/Organism.hpp), [fast CTest definitions](../CMakeLists.txt), [CI workflow](../.github/workflows/build-and-test.yml) |

# Master registry

## 1. Scientific scope: simulate worlds and evolutionary distributions, not one designed alien

**Status:** Frozen architectural commitment.

**Maturity:** Scaffolded: controlled population experiments exist; planetary outcome distributions and uncertainty analysis remain deferred.

### Current/prototype risk
Early prototypes can look like an organism generator or a single-trait/body-plan predictor.

### Mature commitment
AlienEvolution is a computational framework for asking how planetary conditions constrain **distributions of evolutionary outcomes**.

The mature system must support:

- multiple independent evolutionary histories,
- convergence versus contingency analysis,
- uncertainty and confidence,
- distributions such as `P(adaptation | environment)`,
- explicit distinction between established evidence, mechanistically grounded extrapolation, computational approximation, and speculation.

Do not collapse a world to one inevitable organism.

---

## 2. Planet/star inputs and planetary-history engine

**Status:** Required mature capability, implementation open.

**Maturity:** Deferred: Environment still exposes only gravity and dimensionless resource availability; no star/planet history engine exists.

### Current simplification
The active `Environment` currently contains only:

- gravity,
- one dimensionless resource-availability scalar.

### Mature commitment
Users should eventually be able to choose an existing world or define a custom star/planet.

Foundational planetary inputs should propagate causally through:
star/planet parameters
-> planetary physical/chemical history
-> available matter/energy
-> habitat fields and ecological constraints
-> evolutionary dynamics.

Earlier project discussions referred to a Level-1 -> Level-2 structure: foundational planetary inputs produce physically/chemically constrained distributions that become inputs to downstream life/evolution modules.

The mature environment must not remain a pair of organism-level convenience scalars.

---

## 3. Life-possible mode, forced-life mode, and origins of life

**Status:** Frozen architectural commitment.

**Decision boundary:** Frozen product/scientific commitment at the conceptual level; detailed abiogenesis model remains open.

**Maturity:** Deferred: founder initialization is a starting condition, not abiogenesis or probabilistic forced-life support.

### Current simplification
The current evolutionary prototype begins with an already-existing founder organism/program.

### Mature commitment
Support both:

- an ordinary mode that asks whether/when life is physically/chemically plausible,
- an explicitly labeled **forced-life mode** that conditions on life occurring and explores plausible life-hosting histories rather than pretending abiogenesis is known.

Forced-life mode must remain probabilistic, not deterministic.

The mature framework should permit:

- multiple possible origins of life,
- extinction of early origins,
- possibly multiple independent surviving origins where physically plausible,
- uncertainty in abiogenesis transitions.

Do not silently interpret the current founder initialization as a mature origins-of-life model.

---

## 4. Whole biospheres through time

**Status:** Frozen architectural commitment.

**Maturity:** Deferred: the active simulation runs one population in a fixed environment, not a planetary biosphere history.

### Current simplification
The active prototype evolves one isolated population in a fixed simple environment.

### Mature commitment
The eventual output is a planetary biosphere/history, not just one creature.

It should support:

- changing environments through geological/evolutionary time,
- multiple lineages,
- ecological niches,
- geographic ranges,
- radiation,
- migration/dispersal,
- extinction,
- lineage divergence/speciation-like processes,
- ecological interactions,
- representative organisms from different times/places.

The public interface should allow exploration of that history.

---

## 5. HeritableProgram rather than final-trait genome

**Status:** Frozen architectural commitment.

**Decision boundary:** Frozen architecture direction; exact internal representation remains evolvable.

**Maturity:** Scaffolded: V0.3B4 introduces [HeritableProgram](../include/alien_evolution/genetics/HeritableProgram.hpp) ownership of separate regulatory and sensory components; other mature hereditary components remain deferred. [Composition/inheritance tests](../tests/heritable_program_tests.cpp) verify this software boundary, not mature biology.

### Current simplification/history
V0.1 used a seven-parameter genome.
V0.2 replaced that with `RegulatoryProgram`.

### Mature commitment
The mature hereditary object is [Heritable Construction State `G`](HERITABLE_CONSTRUCTION_ARCHITECTURE.md),
controlling construction/regulation through development without requiring DNA.
`HeritableProgram` remains the current compositional compatibility scaffold,
not the final representation of mature heredity (approved S4–S5, issue #21).

M4 ([issue #29](https://github.com/ElliottChambon/AlienEvolution/issues/29)) adds
[HeritableConstructionState](../include/alien_evolution/genetics/HeritableConstructionState.hpp)
as a compatibility scaffold only. `Organism` owns one HCS containing the active
`HeritableProgram` prototype payload; legacy constructors/accessors remain available
and delegate to the same payload. [Fast compatibility tests](../tests/heritable_construction_state_tests.cpp)
verify exact round trips, ownership and reproduction/mutation RNG compatibility.
Current mutation/reproduction and B4–B6 behavior are unchanged. No mature inheritance
chemistry, structural novelty, provenance or dependency mechanism is implemented;
no biological validation is claimed. M5 provenance DAG + causal dependency/invalidation
infrastructure is next.

It must not regress to direct parameters such as:

- leg length,
- eye count,
- body-plan labels,
- explicit organ dimensions.

Current GRN-like regulation is one component, not necessarily the entire mature hereditary system.

---

## 6. Separate heritable SensoryProgram

**Status:** Frozen architectural commitment.

**Authority:** [Approved architecture decision #11](https://github.com/ElliottChambon/AlienEvolution/issues/11).

**Maturity:** Scaffolded: V0.3B4 adds [SensoryProgram](../include/alien_evolution/genetics/SensoryProgram.hpp) ownership and inheritance with a validated runtime adapter. V0.3B5 adds [quantitative sensory mutation](../include/alien_evolution/genetics/SensoryMutation.hpp) of response parameters only; [mutation tests](../tests/sensory_mutation_tests.cpp) and [inheritance tests](../tests/heritable_program_tests.cpp) verify the mechanism, fixed identities, and zero-hazard RNG compatibility. Production development remains legacy clamp-based. See [B5 debt](SCIENTIFIC_DEBT.md#quantitative-sensory-mutation-null-model-v03b5). The [V0.3B6 full controlled experiment](V0_3B6_VALIDATION.md) supports directional selection on sensory half-saturation relative to neutral lines in one frozen resource environment, with direct response assays and no regulatory/structural mutation. Opposite-gradient reversal remains unavailable. This is prototype mechanism evidence, not replacement or biological validation of mature sensing; structural sensory evolution still requires explicit user review.

### Current simplification
V0.3 uses `signalId` plus shifted-Hill `RegulatoryInputChannel` objects.

### Mature commitment
`SensoryProgram` remains the B4–B6 prototype compatibility/scaffold. The
[approved S1–S5 architecture (#21)](https://github.com/ElliottChambon/AlienEvolution/issues/21)
supersedes its framing as the mature endpoint while preserving #11's validated
prototype separation and all B4–B6 evidence.

Mature sensing follows [physical foundations](SENSING_FOUNDATIONS.md) through
networks of [Physical Coupling Processes](PHYSICAL_COUPLING_ARCHITECTURE.md).
A sense is an emergent causal/informational pathway, not a named sense enum or
permanent heritable component label. Heredity/development produces the materials
and structures that couple to physics; it does not directly mutate final responses.
The chain below is the retained prototype boundary, not a universal mature channel:

Causal separation:
environmental quantity
-> sensing/recognition/coupling
-> transduction
-> internal regulatory target
-> regulatory dynamics.

Sensing itself must eventually evolve.

The current ID-based channels are temporary.

Mature sensing should move toward modality-relevant physical/chemical coupling:

- chemical compatibility/recognition rather than arbitrary chemical IDs,
- spectral response rather than named colors,
- mechanical coupling to strain/pressure/contact,
- analogous physical treatment for thermal/electrical/flow/radiation modalities.

Evolution must not jump between unrelated modalities merely by mutating an integer ID.

Future sensor costs/noise/delay/dynamic range/adaptation and exposure/transport constraints must remain possible.

Structural sensor gain/loss/duplication/divergence remains a future user-reviewed architecture checkpoint.

---

## 7. Regulatory dynamics are phenomenological, not universal biology

**Status:** Prototype scientific/numerical debt.

**Decision boundary:** Prototype scientific debt; mature mechanism/model family open.

**Maturity:** Scaffolded: shifted-Hill/RK4 dynamics are numerically tested; mature regulatory-model replacement and empirical calibration remain open.

### Current simplification
Current regulatory dynamics use:

- shifted-Hill response,
- multiplicative combination of regulatory factors,
- first-order production/degradation kinetics,
- nonnegative state clamping in intermediate RK4 states.

### Mature commitment
These mechanisms validate nonlinear heritable regulation and environment-to-regulation coupling; they are not claims that alien life universally uses Earth-style gene regulation or this exact transfer law.

Future work must:

- literature-review appropriate regulatory abstractions,
- compare alternative response/integration laws,
- test numerical convergence and artifact sensitivity,
- calibrate against known systems where relevant,
- preserve chemistry-agnostic alternatives when scientifically defensible.

The nonnegative RK4 stage clamp is a numerical safeguard/debt item and should be revisited when mature state equations/solvers are selected.

---

## 8. DevelopmentalUnit / 3D development

**Status:** Frozen architectural commitment.

**Decision boundary:** Frozen architectural direction; solver details open.

**Maturity:** Deferred: phenotype/development remain a 2D scalar material grid; no 3D DevelopmentalUnit engine exists.

### Current simplification
Development is a 2D scalar material grid.

### Mature commitment
Move to a 3D developmental representation with generalized local entities called `DevelopmentalUnit`, not universally `Cell`.

Potential mature state includes:

- 3D position,
- orientation/polarity,
- lineage/developmental state,
- internal regulatory state,
- material state,
- energy/matter stores,
- sensing,
- secretion/uptake,
- adhesion/contact,
- active mechanical behavior.

Earth-like models may interpret a DevelopmentalUnit as a cell, but alien models must not require terrestrial cellular anatomy.

---

## 9. Current local 8-neighbor signaling is temporary

**Status:** Prototype scientific/numerical debt.

**Maturity:** Scaffolded: the eight-neighbor exponential material measurement remains active in both development paths.

### Current simplification
The current development model computes local material signal from the eight neighboring 2D grid locations with exponential distance weighting.

`neighborhoodLengthScale` is a resolution/model parameter, not a biological constant.

### Mature commitment
Spatial communication should emerge from relevant physical mechanisms such as:

- diffusion/reaction,
- contact,
- transport,
- electrical/mechanical coupling,
- advective flow where relevant,
- other physically defensible information-transfer mechanisms.

Do not preserve the 3x3 kernel as universal developmental biology.

---

## 10. Generic material-deposition effector is temporary

**Status:** Prototype scientific/numerical debt.

**Maturity:** Scaffolded: the bounded single-output deposition effector remains active; physically meaningful action families are deferred.

### Current simplification
A designated regulatory output is converted through a bounded response into scalar material deposition.

`depositionRateScale` is explicitly a prototype developmental abstraction.

### Mature commitment
Regulation should eventually control physically meaningful developmental actions:

- material production/differentiation,
- growth,
- replication/division where applicable,
- secretion/uptake,
- adhesion,
- active forces,
- remodeling,
- transport construction,
- other local behaviors.

Do not optimize the current "one output -> more material" rule into the mature developmental model.

---

## 11. Designated interface/output IDs are temporary

**Status:** Prototype scientific/numerical debt.

**Maturity:** Scaffolded: opt-in external signals coexist with legacy dummy input nodes; designated signal/output identifiers remain prototype couplings.

### Current simplification
Current development names fixed local-material input, resource input, and deposition-output identifiers.

The external-input path is already replacing dummy input nodes, but a designated output node is still used.

### Mature commitment
The mature heritable/developmental architecture should support flexible couplings between inherited programs and multiple developmental behaviors rather than one hard-coded output identity.

Changes to structural sensory/developmental coupling should be introduced through explicit architecture checkpoints rather than arbitrary ID mutation.

---

## 12. Continuous-field physics is only a first validated building block

**Status:** Prototype scientific/numerical debt.

**Decision boundary:** Prototype numerical/scientific debt plus required mature capability.

**Maturity:** Scaffolded: standalone DiffusiveField2D has fast tests but is not coupled to development; mature 3D/multifield physics remains open.

M2 ([issue #25](https://github.com/ElliottChambon/AlienEvolution/issues/25)) adds
[PCP schema vocabulary](../include/alien_evolution/physics/PhysicalCouplingProcess.hpp)
only, verified by [fast schema tests](../tests/physical_coupling_process_schema_tests.cpp).
It records neutral model-owned quantity types, port directionality, separate
state/history/control/observables/ledger categories, M1 metadata and an optional
region association. It selects no physical domain mechanism, executes no PCP,
and provides no biological validation or automatic compatibility. Production
behavior is unchanged and current sensing remains unmigrated. M3 contracts are
scaffolded below; M4 HCS compatibility is scaffolded (entry 5); M5 provenance DAG + causal dependency/invalidation infrastructure is next.

M3 ([issue #27](https://github.com/ElliottChambon/AlienEvolution/issues/27)) adds
[Adaptive Certified Physics contracts](../include/alien_evolution/physics/AdaptiveCertifiedPhysics.hpp),
verified by [fast contract tests](../tests/adaptive_certified_physics_tests.cpp).
These record QoI-specific validity/error/reference/audit/challenge metadata only.
No validity predicate or error estimator is executed, no solver/model selection
occurs, and no scientific mechanism is certified. Numerical/reduction error
declarations remain separate from scientific/model-form uncertainty; reference
edges are not a fidelity ranking or biological truth claim. Challenge cases are
simulator validation artifacts retained in an append-only in-memory registry,
not inherited organism state. Selection-aware policy thresholds remain deferred.
M4 HCS compatibility is scaffolded (entry 5); M5 provenance DAG + causal dependency/invalidation infrastructure is next.

### Current simplification
`DiffusiveField2D` implements:

- a single 2D scalar concentration field,
- nearest-neighbor explicit diffusion,
- no-flux boundaries,
- constant local source,
- first-order decay,
- diffusion followed by source/decay operator splitting.

### Mature commitment
Continuous fields should eventually support the physically required combinations of:

- 3D domains,
- multiple interacting species/quantities,
- secretion and uptake by developing material,
- reactions,
- variable coefficients,
- heterogeneous media,
- boundary/interface conditions,
- advection or flow coupling where relevant,
- conservation checks and unit-consistent quantities.

Exact PDE solvers remain open research/numerical decisions.

The current split diffusion then source/decay update is a first-order operator-splitting debt item and must not silently become the universal field integrator.

The current 2D explicit stability rule should also be generalized/tested for degenerate/domain-specific cases before treating it as a generic field engine.

---

## 13. Materials must differentiate physically

**Status:** Required mature capability, implementation open.

**Maturity:** Deferred: material occupancy has no differentiated physical properties.

### Current simplification
All occupied phenotype material is a single scalar occupancy with no physical differentiation.

### Mature commitment
Material state should eventually include physically meaningful properties such as:

- density,
- stiffness/elasticity,
- viscosity,
- anisotropy,
- failure strength,
- permeability,
- diffusivity,
- thermal/electrical properties,
- chemical reactivity,
- active contractility,
- storage capacity.

Material categories should be defined by properties/functions rather than Earth tissue names.

---

## 14. Mechanics must produce selective consequences

**Status:** Frozen architectural commitment.

**Decision boundary:** Frozen scientific principle; solver choice open.

**Maturity:** Deferred: gravity is stored but no phenotype mechanics solver is implemented.

### Current simplification
Gravity exists in `Environment` but the current phenotype is not mechanically solved.

### Mature commitment
Never use shortcut rules such as:
`high gravity -> short organism`.

Gravity and other mechanical conditions should act through:

- body forces,
- stress/strain,
- failure,
- support requirements,
- locomotor work,
- material cost,
- deformation,
- active forces.

Evolution then responds to those consequences.

Candidate numerical methods remain open and require literature review/benchmarking.

---

## 15. Transport and internal networks

**Status:** Required mature capability, implementation open.

**Maturity:** Deferred: no internal transport network is represented or evaluated.

### Current simplification
The active phenotype has no internal transport network.

### Mature commitment
Sufficiently large/active organisms should face actual transport constraints for quantities such as:

- nutrients/substrates,
- waste,
- gases/chemicals,
- heat,
- signals,
- electrical current,
- internal fluids.

Transport structures should emerge through developmental/evolutionary mechanisms and be evaluated by measurable transport performance rather than resemblance to blood vessels/intestines/etc.

---

## 16. Energetics and resource acquisition are deliberately simplified

**Status:** Prototype scientific/numerical debt.

**Maturity:** Scaffolded: boundary-based acquisition and linear maintenance are active; mature resource/energy accounting is deferred.

### Current simplification
Current energetics use:

- exposed 2D boundary × scalar resource availability × gain coefficient,
- linear maintenance cost per material,
- net energy = acquisition - maintenance,
- fitness = max(0, net energy).

Development also directly multiplies deposition by the same resource-availability scalar.

### Mature commitment
Resource/energy dynamics should eventually be tied to:

- actual environmental distributions,
- acquisition/uptake physics,
- transport,
- chemical/energetic conversion,
- storage,
- maintenance,
- construction costs,
- dissipation/waste,
- limiting substrates,
- conservation constraints,
- task-specific energetic demands.

Fitness is not universally equal to instantaneous energetic surplus; mature reproductive success must emerge from survival/reproduction/ecological consequences.

---

## 17. Fitness and reproduction are prototype population mechanics

**Status:** Prototype scientific/numerical debt.

**Maturity:** Scaffolded: non-overlapping reproduction and parent-selection models are active; mature ecological population mechanics are deferred.

### Current simplification
The current model uses:

- non-overlapping generations,
- fitness-proportional or uniform parent selection,
- fixed requested offspring count,
- zero total fitness -> extinction under fitness-proportional selection (uniform selection is a control mode).

### Mature commitment
Population dynamics should eventually support physically/ecologically meaningful:

- survival,
- reproduction,
- reproductive investment,
- age/life-history where relevant,
- density dependence,
- competition,
- spatial structure,
- migration,
- ecological interactions,
- lineage divergence and extinction.

Do not treat the current selection loop as a universal evolutionary population model.

---

## 18. Mutation model is a controlled null/architecture model

**Status:** Prototype scientific/numerical debt.

**Maturity:** Scaffolded: regulatory mutation events/rates/targets/effects remain tested null models. V0.3B5 composes them with a separate quantitative sensory null model through [HeritableMutation](../include/alien_evolution/genetics/HeritableMutation.hpp). Signal/target IDs and channel structure remain fixed; zero sensory hazard preserves regulatory results and RNG state. Structural co-evolution semantics remain a later user-reviewed checkpoint, with no automatic target repair.

### Current simplification
Current regulatory mutation uses:

- per-opportunity event rates,
- quantitative multiplicative effects,
- uniform target-selection null models,
- network gain/loss/duplication events,
- explicit user-supplied rates rather than biological defaults.

Structural mutation opportunities can bias realized gain/loss counts because the number of eligible absent/present targets differs.

### Mature commitment
Mutation and inheritance should progressively become tied to the chosen heritable/replication architecture and calibrated/mechanistic evidence where available.

Requirements include:

- explicit mutation opportunity models,
- documented target biases,
- sensitivity analysis,
- no hidden assumption that today's null model is universal,
- coordinated mutation across components of `HeritableProgram`,
- eventual evolvable inheritance machinery/mutation spectra where physically
  justified; numerical/scientific solver fidelity remains simulator-owned.

Current structural-opportunity bias must be considered when interpreting apparent evolution of network complexity.

---

## 19. Function must be measured, not assigned by anatomy labels

**Status:** Frozen architectural commitment.

**Maturity:** Scaffolded: material, exposed-boundary, and energetic metrics exist; mature physical functional analysis is deferred.

### Current simplification
The prototype mostly measures scalar material/boundary/energy quantities.

### Mature commitment
Structures should be evaluated by physical contribution:

- load bearing,
- force/work,
- locomotion,
- flux/transport,
- exchange,
- thermal control,
- sensing/information,
- resource acquisition,
- protection,
- reproduction.

The simulator should not require labels like Leg, Eye, Skeleton, Intestine, BloodVessel before function can be measured.

Earth terminology may be used later as user-facing analogy.

---

## 20. Geometry/topology must emerge and remain separate from rendering

**Status:** Frozen architectural commitment.

**Maturity:** Scaffolded: the scientific phenotype is a 2D grid; mature 3D topology and derived rendering remain deferred.

### Current simplification
The scientific phenotype is currently a 2D occupancy grid.

### Mature commitment
Scientific phenotype state must eventually support:

- 3D topology,
- cavities,
- branching/fusion/splitting,
- internal/external boundaries,
- material interfaces,
- transport networks,
- geometry appropriate for physics.

Visualization geometry is a separate derived layer.

The scientific engine must not store or evolve decorative render meshes as if they were biology.

---

## 21. On-demand representative-organism reconstruction

**Status:** Frozen architectural commitment.

**Maturity:** Deferred: evaluated organisms store simple phenotypes; no versioned reconstruction/archive pipeline exists.

### Current simplification
The current prototype directly stores a simple phenotype per evaluated organism.

### Mature commitment
Large evolutionary histories should archive enough scientific state/metadata to deterministically reconstruct/refine representative organisms on demand.

Archive candidates include:

- heritable program,
- lineage identity,
- environment,
- random seed,
- model version,
- developmental configuration,
- material/topology summaries,
- functional measurements,
- intermediate-resolution state.

High-resolution visualization can be reconstructed and cached.

Presentation-only inferred detail must remain distinguishable from simulation-grounded structure.

---

## 22. Adaptive multiscale fidelity is a core scalability mechanism

**Status:** Frozen architectural commitment.

**Decision boundary:** Frozen architecture direction; exact methods open.

**Maturity:** Deferred: representations are fixed; adaptive reductions, promotions, and state/conservation mappings are not implemented.

### Current simplification
Current development/physics uses fixed representations.

### Mature commitment
Use the cheapest representation that remains valid for the local physical regime.

Potential reduced models include:

- beams/rods,
- shells/membranes,
- reduced flow networks,
- continuum coarse-graining,
- simplified diffusion/transport.

Every reduction requires:

- explicit validity conditions,
- error tolerances,
- conversion rules,
- conservation/state mapping,
- promotion criteria,
- reduction criteria,
- cross-fidelity benchmarks.

If assumptions fail, promote back to a more general representation.

Evolution must not be allowed to exploit low-fidelity numerical artifacts.

---

## 23. Cross-fidelity and scientific validation hierarchy

**Status:** Frozen architectural commitment.

**Maturity:** Scaffolded: fast tests and controlled experiments verify prototype mechanisms; empirical and cross-fidelity validation remain deferred.

### Current simplification
Most existing experiments are architecture/mechanism validation of simplified models.

### Mature commitment
Distinguish at least:
1. software/numerical correctness,
2. mechanistic correctness,
3. calibration to known systems,
4. withheld-data empirical validation,
5. cross-system generalization,
6. prospective prediction where possible,
7. alien extrapolation with explicit uncertainty.

Cross-fidelity comparisons are required wherever reduced models are used.

Do not call architecture-level tests "biological validation."

---

## 24. Literature-driven model replacement and versioning

**Status:** Frozen architectural commitment.

**Maturity:** Scaffolded: M1 ([issue #23](https://github.com/ElliottChambon/AlienEvolution/issues/23)) adds standalone [scientific metadata records](../include/alien_evolution/science/ScientificModelMetadata.hpp) for exact P1/B1/B2/M/X/S codes, layered evidence/literature, model identity/version, inert supersession/alternatives, parameters/units, assumptions/validity scope, and distinct numerical/reduction versus scientific/model-form uncertainty. [Fast tests](../tests/scientific_metadata_tests.cpp) verify metadata and syntactic validation only. This software infrastructure selects no scientific mechanism, provides no biological validation, and does not change production simulation behavior. Each mature replacement still needs its own review and evidence; M2 schema vocabulary is scaffolded (entry 12); M3 contracts are scaffolded (entry 12); M4 HCS compatibility is scaffolded (entry 5); M5 provenance DAG + causal dependency/invalidation infrastructure is next.

### Current simplification
Many early mechanisms are selected because they are useful architecture tests.

### Mature commitment
Fundamental mechanism choices should be:

- literature-reviewed,
- compared against alternatives,
- benchmarked,
- sensitivity-tested,
- versioned,
- associated with explicit evidence/uncertainty.

Scientific rigor takes priority over preserving obsolete code.

When a validated replacement makes production code obsolete, remove it and preserve history through Git/docs/benchmark records.

---

## 25. Outer ensembles and convergence/contingency experiments

**Status:** Frozen architectural commitment.

**Maturity:** Scaffolded: controlled replicated experiments are building blocks; the mature planetary-history ensemble framework is deferred.

### Current simplification
Current validation runs small controlled population experiments.

### Mature commitment
The framework must support many independent histories under identical or systematically varied conditions.

Use outer ensembles/Monte Carlo-style experimental replication to estimate distributions, sensitivity, convergence, contingency, and relationships such as `P(adaptation | environment)`.

Do not confuse an inner stochastic numerical method with the outer evolutionary-history ensemble.

---

## 26. AI/agent experimentation is future tooling, not a model requirement

**Scope:** Deferred optional tooling, not core architecture.

**Maturity:** Deferred optional tooling: no scientific commitment depends on an AI/agent experimentation subsystem.

### Current simplification/preference
The project currently favors interpretable numerical/statistical methods and avoids AI/ML unless clearly useful/easy.

### Mature direction
Future agents may run large experiment suites, search correlations, suggest hypotheses, and summarize results.

Guardrails:

- agents must not silently change scientific assumptions,
- all runs remain reproducible/versioned,
- discovered patterns require statistical/scientific validation,
- AI output is not itself biological evidence.

This should remain future-only unless it clearly improves the research workflow.

---

# S1–S5 architecture freeze (issue #21)

**Authority:** [User-approved issue #21](https://github.com/ElliottChambon/AlienEvolution/issues/21).
These commitments supersede older mature-endpoint language in entries 5–6 and
clarify entries 13, 18, 22–24. They do not invalidate prototype evidence.

| Commitment | Frozen architectural contract | Required mature capability / implementation open | Maturity audit |
| --- | --- | --- | --- |
| S1 | Physics-first sensing; influence, detectability, internal integration, and adaptive relevance separate; primitive observables distinct from inference; no fixed named senses | Propagation/access, noise/information, internal use, and consequences for specific mechanisms | Deferred; B4–B6 scalar channels are prototype scaffolds |
| S2 | PCP is fundamental; typed bidirectional ports, state/history, observables, control, exchange ledger; channels only validated reductions; evolution changes phi, simulator controls m | Domain-specific deterministic/stochastic/hybrid/quantum realizations; Adaptive Certified Physics and selection-aware adversarial validation | Scaffolded: M2 schema vocabulary and M3 inert contracts; no PCP execution or certification engine exists |
| S3 | Many-to-many physical coupling mechanisms/observables; geometry transforms information; polymodal structures allowed; Earth receptors are evidence | Versioned mechanisms, unresolved alternatives, layer-specific P1/B1/B2/M/X/S provenance and validation | Deferred; no mature coupling-mechanism catalog is implemented |
| S4 | G -> development -> material state/geometry -> Material Compiler -> response models -> PCPs -> consequences -> reproduction; compilers/caches never inherited | Chemistry-agnostic construction representation, coupled development/material response, mutational-accessibility audits | Scaffolded: M4 HCS owns the current HeritableProgram compatibility payload; mature construction/inheritance remains open |
| S5 | Transformation System (G,I,M,P); physical opportunities and footprints, no automatic repair; separate ancestry/state/function; immutable provenance DAG distinct from derived dependency graph | Mechanism-dependent transformations, multi-parent history, incremental invalidation, novelty/innovation/adaptation diagnosis | Deferred; current regulatory/sensory mutation remains a null-model prototype |

The detailed contracts are [S1/S3](SENSING_FOUNDATIONS.md),
[S2](PHYSICAL_COUPLING_ARCHITECTURE.md), and [S4/S5](HERITABLE_CONSTRUCTION_ARCHITECTURE.md).
Frozen architecture is separate from required mature capability: publication of
these documents implements M0 only. No solver, material chemistry, inheritance
backend, mutation kernel, probability assignment to unresolved models, or
biological validation is frozen by this milestone. Mechanism-specific checkpoints
and literature/benchmark review remain required at M7+.

Every mechanism/architecture record distinguishes stable causal interfaces,
replaceable mechanism models, unresolved alternatives, prototype/reduced
abstractions, evidence/provenance, validity domain, and version/supersession.
High numerical fidelity does not establish scientific truth. Evolutionary
challenge cases must be retained and failed certified domains repaired/shrunk.
Pruning cannot remove a coupling's future evolutionary possibility.

# Maintenance rule

Whenever a simplified implementation is introduced, removed, or replaced:

1. update `SCIENTIFIC_DEBT.md`,
2. update this mature-commitment registry if the mature target changes,
3. update `MATURE_ARCHITECTURE.md` when a fundamental architecture decision is frozen,
4. update `ROADMAP.md` when sequencing changes,
5. record validation/benchmark evidence,
6. do not close a mature commitment merely because a prototype mechanism compiles and passes unit tests.

The registry should explicitly mark which commitments have been fully replaced/validated versus merely scaffolded.

For each progress update, identify the implementation/model version, supported
regime, remaining limitations, and linked benchmark/validation record. Mark
`Replaced and validated` only for the capability actually demonstrated; retain
any residual debt. The registry is an audit of the mature target, not permission
to implement deferred architecture or to skip an explicit review checkpoint.
