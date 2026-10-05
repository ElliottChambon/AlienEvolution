# AlienEvolution Scientific Benchmark Suite

## Purpose

AlienEvolution should not select mature developmental or physical models based
only on visual output or programming convenience.

Candidate models must demonstrate that they can reproduce the classes of
behavior required by the mature architecture.

Benchmarks should eventually include quantitative reference problems,
convergence tests, cross-model comparisons, and experimental comparisons
where appropriate data exist.

---

# Benchmark 1 — Founder Growth

Start from a minimal founder state.

Requirements:

Development proceeds without a predefined final geometry.

Material remains connected unless a physical/developmental mechanism causes
separation.

Mass and energy conservation rules behave as specified.

No biological material spontaneously appears outside permitted developmental
mechanisms.

---

# Benchmark 2 — Symmetry Breaking

Begin from a nearly symmetric developmental state.

Requirements:

The system must be capable of generating stable developmental axes or other
asymmetries through an explicitly modeled mechanism.

The mechanism must distinguish deterministic symmetry breaking from
stochastic symmetry breaking.

Final orientation should not be secretly hard-coded into geometry.

---

# Benchmark 3 — Material Differentiation

A developing organism must be able to generate regions with different
material states.

Requirements:

Regions may differ in physical properties.

Differentiation follows regulatory/developmental mechanisms.

Material identity affects subsequent mechanics or transport.

---

# Benchmark 4 — Cavity Formation

The model must permit development of a stable internal hollow region.

Requirements:

The cavity is produced by development rather than pre-built geometry.

Internal and external surfaces remain distinguishable.

The cavity may change shape during subsequent growth.

---

# Benchmark 5 — Branching Morphogenesis

A developing structure must be capable of forming multiple branches.

Requirements:

Branch count and geometry are developmental outcomes.

The model must permit branches to compete, terminate, or continue.

Branch formation should be able to respond to relevant signals or physical
conditions.

---

# Benchmark 6 — Topology Change

The representation must permit processes such as:

fusion,
splitting,
closure,
perforation,
detachment,
extrusion.

Requirements:

Topology change does not require manually rebuilding the entire simulation.

Mass/material accounting remains valid.

---

# Benchmark 7 — Differential Growth and Buckling

Create adjacent regions with different growth behavior.

Requirements:

Differential growth generates physically meaningful deformation.

The model permits bending, folding, or buckling where mechanics predicts it.

Results should be compared against analytical or established numerical
solutions when available.

---

# Benchmark 8 — Load-Driven Remodeling

Apply sustained mechanical loading to a developing structure.

Requirements:

A mechanosensitive developmental model can alter local material or geometry.

Remodeling changes the resulting stress distribution.

Removing or changing the load changes the developmental outcome when the model
predicts such dependence.

---

# Benchmark 9 — Demand-Driven Transport Growth

Create spatially heterogeneous resource or transport demand.

Requirements:

A transport-capable developmental system can respond to unmet demand.

Network geometry may branch, widen, regress, or reorganize.

Transport performance must be measured directly rather than inferred from
appearance.

---

# Benchmark 10 — Multi-Function Geometry

Generate structures with similar gross geometry but different physical roles.

Example comparison:

load-bearing elongated structure,
transport conduit,
active locomotor appendage,
sensory projection.

Requirements:

Material, geometry, internal organization, and mechanics diverge as a
consequence of function.

The simulator must not require the structures to be assigned Earth-organ
names.

---

# Benchmark 11 — Sparse-to-Dense Development

A structure begins as a small number of DevelopmentalUnits and later becomes a
large dense region.

Requirements:

The simulation remains stable across the transition.

If coarse-graining is used, functional properties remain consistent within
defined tolerance.

---

# Benchmark 12 — Adaptive Fidelity Reduction

Begin with a general high-fidelity physical representation.

Allow a region to enter a regime where a reduced model is mathematically
appropriate.

Example:

3D elongated structure
→ beam representation.

Requirements:

The solver verifies reduction criteria before conversion.

Relevant state is preserved.

The reduced model agrees with the detailed model within predefined tolerance.

---

# Benchmark 13 — Fidelity Promotion

Take a region currently represented by a reduced model and drive it outside
the assumptions of that model.

Example:

beam
→ severe local deformation or branching.

Requirements:

The system detects failure of the reduced-model assumptions.

The region is promoted to a more general representation.

No major discontinuity in conserved or functionally important quantities is
introduced by the transition.

---

# Benchmark 14 — Cross-Fidelity Evolution

Create competing phenotypes A and B.

Evaluate them under both low- and high-fidelity representations.

Requirements:

Fitness-relevant ordering should remain consistent within the validated regime.

If fidelity changes reverse an evolutionary conclusion, the low-fidelity model
must not be considered validated for that regime.

This benchmark is essential because evolutionary optimization can exploit
numerical approximation errors.

---

# Benchmark 15 — Reproducibility

Run an identical simulation twice with the same:

configuration,
seed,
environment,
model version,
and initial state.

Requirements:

The same deterministic outputs are reproduced where deterministic behavior is
expected.

Stochastic components must still be reproducible from the recorded seed.

---

# Benchmark 16 — Environmental Counterfactual

Hold hereditary state constant while altering one environmental parameter.

Requirements:

Phenotypic and functional changes must arise through modeled causal mechanisms.

The environment must not directly overwrite final organism traits.

---

# Benchmark 17 — Heritable Counterfactual

Hold the environment constant while varying hereditary parameters.

Requirements:

Changes propagate through development.

The resulting phenotype is an outcome rather than a direct encoded trait.

---

# Benchmark 18 — Visualization Reconstruction

Archive a simulated representative organism.

Reconstruct it later for high-resolution inspection.

Requirements:

The reconstructed organism preserves simulation-grounded topology, material
identity, and major functional relationships.

Visualization must not silently introduce biologically meaningful structures
that were absent from the archived phenotype.

---

# Controlled prototype benchmark — sensory-response evolution (V0.3B6)

The [B6 validation record](V0_3B6_VALIDATION.md) tests whether selection shifts
inherited resource sensitivity along a deterministic phenotype-fitness gradient.
Internal regulation, wiring, fold change, and cooperativity are fixed; only
half-saturation mutates. Thirty matched replicates compare selected mutation,
neutral mutation, and selected no mutation, with standardized response curves
and independent final phenotype assays.

The first full local run supports the lower-K prediction at resource 2: paired
adaptive shift 0.481431, sample SD 0.396876, approximate 95% mean interval
[0.339411, 0.623452], positive/negative/tie counts 25/5/0. No extinctions or
development failures occurred. No opposite-gradient candidate existed, so reversal
and reciprocal-environment validation remain unavailable. Parameters were frozen
before evolution and were not tuned after outcomes.

This is mechanism evidence in a 2D, shifted-Hill, boundary-energy prototype,
not biological calibration or closure of a mature benchmark. Mature sensing
remains scaffolded; structural sensory evolution requires its architecture
checkpoint. The executable is a manual milestone run outside CTest/CI; only
small deterministic fixture tests join the fast suite.

---

# CHEM-1 mature migration benchmark: M7A reductions + M7B isolated pair + M7C finite bath

[CHEM-1](CHEMICAL_COUPLING_ARCHITECTURE.md) is generic physical association
`L + S <-> LS`, distinct from the controlled V0.3B6 sensory-selection prototype.
Its ideal 3D spherical reference has finite intrinsic reactivity; the sphere is
benchmark geometry, not anatomy. M7A implements analytical rates/equilibrium and
deterministic/stochastic well-mixed constant-reservoir reductions only. It supplies
no biological calibration, evolutionary evidence or production sensing replacement.

The effective Markov reduction assumes rapid local rebinding can be integrated
out with negligible rebinding interference; ideal spherical geometry alone does
not establish validity. Frozen spatial configurations are an open/reservoir
reactive sphere for association rates and a closed finite spherical domain with
reflecting outer boundary for depletion/conservation. M7B now implements the exact
**isolated-pair, unbounded-3D radial Green-function kernel** for the same SCK/
back-reaction mechanism. M7C adds the closed finite **competitive capacity-one**
bath: N conserved ligands in a concentric reflecting shell, radial spectral
propagation, conditional loser updates, intrinsic dissociation back to r=a, and
exact ligand-count bookkeeping. An explicit maintained open-reservoir particle
source remains unimplemented and is physically distinct from both references.

The dedicated fast `ReversibleChemicalAssociationTests` checks input validation,
chi sweep/limits/crossover, rebinding/effective-rate consistency, occupancy bounds,
exact relaxation, equal affinity with different response times, seeded CTMC
trajectories and stationary mean/variance. The statistical tolerance is fixed:
12,000 trajectories, seed 3502, 16 relaxation times, absolute tolerance 0.025.
M1/M2/M3 declarations and eight challenge entries are checked. B4–B6 and M6
retain their separate scope.

M7B adds dedicated fast spatial-reference tests. They check dimensionless conversion,
the characteristic cubic/Vieta relations including complex-conjugate roots, independent
high-precision A12/A3 density fixtures spanning reaction/diffusion/dissociation regimes,
nonnegative real physical density, deterministic evaluation, the exact irreversible
limit, numerical-conditioning rejection, and the reference's narrow M1/M2/M3 scope.

The existing eight M7A spatial challenge entries remain declared **UNRESOLVED at
their own QoIs**. M7B currently references radial volume/shell density, not the
reservoir `k_on`, occupancy-response/variance, depletion or finite-domain QoIs
needed to close those challenges. The first reference kernel therefore enables
future cross-fidelity work without silently marking non-overlapping benchmarks passed.
M7C now provides overlapping finite-bath conservation/occupancy-trajectory QoIs,
but it does not automatically certify every M7A challenge. Open-reservoir dynamics,
arbitrary geometry and broader many-particle chemistry still need separate reviewed
references where their QoIs matter.

M7C dedicated fast tests verify Robin-Neumann/Neumann-Neumann spectral roots,
independent high-precision survival/first-reaction/density/CDF fixtures, exact mean
first-reaction time, finite-system equilibrium, CDF bounds/monotonicity, explicit
spectral-resolution failure, seeded competitive trajectories and capacity-one
conservation. The larger N/chi/delta/Lambda experiment is built as
`AlienEvolutionChem1CFiniteBathValidation` but deliberately excluded from CTest.

M7C-R1 additionally verifies that query-time active mode counts equal the compiled
budget at `tau_min`, decrease monotonically (or remain equal) as query time grows,
never exceed the compiled table, and retain the minimum numerical mode policy.
The existing high-precision fixtures remain the correctness guard: cost adaptation
is not allowed to change the declared physical/numerical result.

A focused post-M7C resolution scan tested the previously difficult
`chi in {1,10}`, `delta in {1,10}`, `N in {1,2,5}` regimes with ten seeded
trajectories per cell. Resolved trajectories increased from **46/120** at
`tau_min=1e-4` (176 reactive modes) to **70/120** at `1e-5` (552 modes) and
**81/120** at `1e-6` (1743 modes). The `chi=10, delta=10` cells remained mostly
or completely unresolved even at `1e-6`. This is a numerical-resolution study,
not a statistical equilibrium validation; ten trajectories per cell are
insufficient for occupancy inference. The result supports retaining the current
library default floor rather than attempting to solve the short-time gap solely by
brute-force spectral depth.

The validity-boundary challenge is onset of rebinding interference/spatial memory
inside ideal-dilute chemistry. Nonideal thermodynamic promotion cannot be certified
by the ideal-dilute SCK reference; it and nonspherical supports remain separate
future reference/extension work. M7B is an exact reference for the chosen SCK
isolated-pair model, not proof that SCK is the universal microscopic chemistry model.

# Acceptance Philosophy

A candidate numerical or developmental model does not need to solve every
benchmark at the same fidelity.

Instead, AlienEvolution may combine specialized models.

However:

every mature component must clearly state which regimes it supports,

every simplification must state its validity conditions,

overlapping models must be cross-validated,

and the combined architecture must collectively support the full benchmark
suite.


# MAT-1 material/compiler calibration

M8A adds MAT-1 as the first executable Material Compiler calibration. Two abstract
benchmark phases form a perfectly bonded periodic one-dimensional laminate.

For loading in the layer plane under the explicitly scalar iso-strain calibration:

```text
E_parallel = f_A E_A + f_B E_B
```

For loading normal to the layers under the explicitly scalar iso-stress series
calibration:

```text
1/E_normal = f_A/E_A + f_B/E_B
```

These are derived directly from compatibility/force equilibrium for the stated
one-dimensional boundary-value problems. They are not asserted to be general 3-D
laminate Young's moduli.

Fast tests verify:
- pure-phase and equal-phase limits;
- arithmetic/harmonic references;
- phase-label/fraction exchange invariance;
- linear modulus scaling;
- harmonic <= arithmetic for unequal positive phases;
- direction normalization and explicit oblique-query refusal;
- scale/period diagnostic retention without inventing an RVE threshold;
- versioned Material Compiler update-point/dependency/cost provenance;
- generic material state contains scale/component descriptors rather than compiled
  effective-property fields.

Full 3-D laminate mechanics, Hill-Mandel/RVE solver validation and material-field
integration are later benchmarks.
