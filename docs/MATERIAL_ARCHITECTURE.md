# Material architecture

## Status

M8A freezes the first mature material-state / Material Compiler architecture and
adds one executable calibration, MAT-1. This document is a scientific architecture
record, not a claim that mature materials, mechanics or homogenization are complete.

The central rule is:

```text
material identity = physical state at a declared scale
effective properties = compiled responses to a declared query
```

A material is therefore **not** a permanent property sheet containing Young's
modulus, conductivity, permeability, diffusivity, strength, etc. Those quantities
are responses that may depend on scale, direction, thermodynamic state, loading
regime, history and the particular physical question.

---

## Mature causal placement

```text
Heritable Construction State G
        |
        v
development
        |
        v
developed material state M_l(x,t) + geometry/topology
        |
        v
Material Compiler
        |
        +--> mechanical constitutive response
        +--> transport response
        +--> thermal response
        +--> electrical/optical/etc. response
        |
        v
PCPs / organism physics / consequence / ecology / reproduction
```

Evolution changes construction/developmental causes. The Material Compiler and its
caches are simulator-owned derived artifacts and are never inherited.

---

## Approved M8 architecture decisions

### M8-A1 — State, not property catalog

Material identity is represented by physical state. Effective material properties
are derived responses and do not define the material itself.

### M8-A2 — Declared scale

Every developed material representation has a declared spatial/coarse-graining
scale. A response compiled at one scale is not automatically valid at another.

### M8-A3 — Conserved matter / thermodynamic foundation

Composition/amount and thermodynamic state form the physical foundation. Mature
representations must remain compatible with the relevant conservation laws and
thermodynamic bookkeeping.

### M8-A4 — Extensible state components

Phase, microstructure, orientation, interfaces, porosity/connectivity and justified
internal variables are extensible state components. They are not a fixed catalog of
Earth material classes.

M8A intentionally does **not** freeze one universal C++ payload containing every
possible state variable. The stable envelope identifies the state representation
and versioned components; numeric state remains representation-specific.

### M8-A5 — Effective properties are derived

Effective modulus, conductivity, diffusivity, permeability, yield strength and
similar quantities are not heritable primitives. They may appear in compiled
response models inside declared validity domains.

### M8-A6 — Query-dependent response

The Material Compiler answers a response query. Its output depends on material
state, query/QoI, scale/regime and available scientific models. It does not produce
one context-free permanent property sheet.

### M8-A7 — No hidden physical memory

If damage, plastic strain, phase fraction, defect state, orientation or another
history-bearing quantity changes future physics, it belongs in explicit material
state/internal state or a declared history representation. Compiler caches may
store numerical work but may not hide physical state.

### M8-A8 — Thermodynamic admissibility

Conservation and thermodynamic admissibility constrain constitutive models.
Free-energy/dissipation-potential or generalized-standard-material formulations
are useful where justified, but no one formalism is universal.

### M8-A9 — Objectivity and material symmetry

Continuum mechanical models must satisfy the appropriate observer/objectivity and
material-symmetry requirements in their declared regime.

### M8-A10 — Structural anisotropy

Anisotropy should arise from physical structure/orientation/symmetry. It is not a
universal scalar evolvable "anisotropy" property.

### M8-A11 — Interfaces/connectivity are physical state

Equal phase fractions need not imply equal response. Interface area, morphology,
connectivity, topology and orientation can materially change mechanics, transport,
thermal response and failure.

### M8-A12 — Homogenization is conditional

The Material Compiler may return **not homogenizable**. A caller cannot require an
effective local property when scale separation/statistical representativity is not
scientifically justified.

### M8-A13 — RVE/SVE adequacy is QoI-specific

There is no universal RVE size attached permanently to a material. Representativity
must be established for the quantity/regime being compiled.

### M8-A14 — Bounds before expensive physics

When rigorous or well-justified bounds/cheap estimates already determine the
scientific/evolutionary conclusion, the compiler should not perform an unnecessary
full-field solve.

### M8-A15 — Micro/macro energetic consistency

Where continuum mechanical homogenization is used, Hill-Mandel-type micro/macro
work/energy consistency is required as appropriate to the model.

### M8-A16 — Promote when homogenization fails

Explicit microstructure, nonlocal response or another higher-detail representation
must remain available when a local homogenized material is not adequate.

### M8-A17 — Simulator-owned compiler

The Material Compiler, compiled constitutive responses, material atlases, caches,
surrogates and local trust-region approximations are simulator artifacts, not
organismal inheritance.

### M8-A18 — Evolution changes causes

Evolution changes construction/material causes. Mature mutation does not directly
change compiled E, k, D, strength, etc. unless a future reviewed representation
shows that a parameter is itself the correct physical construction variable.

### M8-A19 — Versioned scientific update points

Materials science is expected to change. The stable architecture therefore exposes
explicit versioned update points rather than burying current science in heredity.

M8A stable update-point keys:

```text
constituent_physics
phase_thermodynamics
microstructure_evolution
homogenization
constitutive_response
numerical_realization
calibration_reference
```

These are open string identifiers, not a closed enum. Future science may add new
stages.

### M8-A20 — Compiled response provenance

A compiled response should retain:
- source material-state identity;
- query/QoI and scale context;
- compiled model identity/version;
- ACP validity/error declarations;
- scientific update-point bindings;
- dependency/invalidation identities;
- simulator cost information;
- provenance sufficient to reproduce/review the result.

### M8-A21 — Separate uncertainty categories

Numerical/reduction uncertainty is not scientific/model-form uncertainty, and
neither is uncertainty in the underlying material state. These must remain
distinguishable.

### M8-A22 — Cost after adequacy

Scientific adequacy is a hard constraint. Among scientifically adequate responses,
the engine should use the cheapest appropriate representation.

---

## DevelopedMaterialStateEnvelope

M8A implements a stable envelope around a versioned material-state representation.

The envelope records:

- a material-state identity;
- state representation model identifier/version;
- coarse-graining length;
- optional characteristic microstructure length;
- ordered versioned component descriptors.

A component descriptor has:

- an open component key;
- a versioned representation model;
- representation scale;
- optional description.

The generic envelope deliberately carries **no universal modulus, conductivity,
diffusivity or strength fields**.

Example conceptual envelope:

```text
state: material.example.42
representation: future.multiphase_state:v3
coarse scale: 1 mm
microstructure scale: 20 um

components:
  composition      -> chemistry.multiphase_composition:v5
  phase            -> thermo.phase_state:v2
  microstructure   -> structure.two_point_plus_interfaces:v4
  orientation      -> structure.orientation_tensor:v1
  internal_state   -> mechanics.damage_state:v7
```

The actual numeric payload of each component belongs to its representation-specific
state type/backend, allowing future scientific models to evolve without changing
the stable envelope or heredity architecture.

M8A does not yet define a universal distributed material field over organism
geometry.

---

## Material Compiler contract

M8A adds simulator-owned compilation vocabulary, not a universal runtime material
compiler engine.

A material response query declares:

- query identity;
- response family;
- quantity-of-interest key;
- optional spatial scale;
- optional temporal scale;
- optional regime identifier.

A compilation returns one of:

```text
compiled
not_homogenizable
unsupported_query
scientifically_unresolved
```

This distinction is essential. Failure/refusal is a valid scientific answer.

A successfully compiled record must identify:
- the compiled model;
- a matching AdaptivePhysicsContract whose QoI covers the query;
- scientific update-point bindings;
- source/dependency identities;
- any declared uncertainty in the underlying material state;
- optional simulator cost descriptors.

`MaterialStateUncertaintyDescriptor` is intentionally separate from ACP
numerical/reduction error declarations and from scientific/model-form uncertainty.
MAT-1 uses deterministic benchmark state and therefore carries no state-uncertainty
entry, but later measured/inferred material states can declare one without
reclassifying it as solver error.

An uncompiled record cannot silently carry a "certified" compiled model.

M8A implements no automatic model selection, promotion, cache, homogenization
threshold or execution engine.

---

## Scientific updateability

The update-point structure is intended to make literature changes local.

Conceptually:

```text
Developed Material State
        |
        +--> U1 constituent_physics
        |
        +--> U2 phase_thermodynamics
        |
        +--> U3 microstructure_evolution
        |
        +--> U4 homogenization
        |
        +--> U5 constitutive_response
        |
        +--> U6 numerical_realization
        |
        +--> U7 calibration_reference
        |
        v
Compiled response
```

If, for example, a new homogenization model supersedes U4:
1. heredity does not change;
2. developed state does not need to change unless the new science requires
   additional state variables;
3. U4 model/version is replaced or added as an alternative;
4. dependent compiled responses are invalidated through causal dependencies;
5. cross-fidelity/challenge validation is rerun;
6. evolutionary conclusions are compared before the replacement is trusted.

---

## Scale, RVE and homogenization

RVE/SVE literature treats representativity as a scale/statistics problem rather
than a universal material constant. A finite heterogeneous sample may approach an
RVE only approximately, and adequacy can differ by QoI.

Mature compiler logic should therefore be able to progress through levels such as:

```text
cheap physical bounds
       |
       v
mean-field / analytical estimate
       |
       v
statistical SVE/RVE
       |
       v
full-field microstructure solve
       |
       v
explicit/nonlocal microstructure if homogenization fails
```

The exact sequence is mechanism/QoI dependent; it is not a scalar fidelity ladder.

Mechanical homogenization must preserve appropriate micro/macro energetic
consistency such as the Hill-Mandel condition.

No universal scale-separation threshold is selected in M8A.

---

## MAT-1: first executable calibration

### Purpose

MAT-1 demonstrates the architectural statement:

```text
same constituent phases + same fractions
        +
different structural orientation / query
        =
different derived response
```

without implementing a full mechanics solver.

### State

MAT-1 is a perfectly bonded periodic two-phase one-dimensional laminate.

Each abstract benchmark phase has a positive scalar one-dimensional axial elastic
modulus:

```text
phase A: E_A
phase B: E_B
```

The MAT-1 developed state includes:
- phase identities;
- phase-A fraction f_A and phase-B fraction 1-f_A;
- laminate period;
- unit layer normal;
- composition, periodic-microstructure and orientation component descriptors.

These constituent moduli are benchmark/reference inputs. They are **not** mature
heritable material properties.

### Exact in-plane iso-strain case

For loading in the layer plane in the MAT-1 one-dimensional calibration, perfect
compatibility imposes equal axial strain:

```text
epsilon_A = epsilon_B = epsilon
```

Average stress is:

```text
sigma = f_A E_A epsilon + f_B E_B epsilon
```

therefore:

```text
E_parallel = f_A E_A + f_B E_B
```

This arithmetic response is exact for this stated scalar iso-strain problem.

### Exact layer-normal iso-stress case

For loading normal to the layers in the MAT-1 one-dimensional series calibration,
force equilibrium imposes equal axial stress:

```text
sigma_A = sigma_B = sigma
```

Average strain is:

```text
epsilon = f_A sigma/E_A + f_B sigma/E_B
```

therefore:

```text
1/E_normal = f_A/E_A + f_B/E_B
```

This harmonic response is exact for this stated scalar iso-stress problem.

### Critical limitation

These two scalar formulas are **not** declared to be the general three-dimensional
Young's moduli of an isotropic-phase laminate. Full 3-D elasticity includes tensor
coupling, Poisson effects and boundary-condition dependence. Exact layered-composite
literature treats the full stiffness/compliance tensor.

MAT-1 refuses oblique loading rather than interpolating between the two scalar
special cases.

### Scale

MAT-1 records the laminate period and, when requested, the response-scale/period
ratio. It represents the periodic homogenized calibration limit and deliberately
does not select a finite-scale RVE threshold.

---

## MAT-1 verification

M8A tests:

- material-state scale/component validation;
- open/extensible update-point identifiers;
- compilation disposition invariants;
- source/dependency retention;
- simulator-owned cost metadata;
- phase-fraction and positive-modulus validity;
- direction normalization;
- arithmetic in-plane response;
- harmonic layer-normal response;
- pure-phase limits;
- equal-phase limit;
- A/B exchange invariance;
- linear scaling with constituent moduli;
- harmonic <= arithmetic for unequal positive phases;
- explicit refusal of oblique queries;
- finite-scale representativity guardrails;
- MAT-1 metadata/provenance/update-point bindings.

---

## Literature basis

### Scale / RVE / heterogeneous structure

- Ostoja-Starzewski (2006), *Material spatial randomness: From statistical to
  representative volume element*. The paper emphasizes scale separation,
  SVE-to-RVE convergence and boundary-condition-dependent bounds:
  https://doi.org/10.1016/j.probengmech.2005.07.007
- Doškář et al. / review of 3-D RVE generation methods across heterogeneous
  materials:
  https://doi.org/10.1016/j.pmatsci.2018.02.003
- Zubov et al. (2024), modern discussion of REV metrics, stationarity and the fact
  that representativity depends on the property/metric:
  https://doi.org/10.1016/j.advwatres.2024.104762

### Layered composites / homogenization

- Pindera et al. (2012), exact solution for periodic layered composites:
  https://doi.org/10.1016/j.mechrescom.2012.08.007
- Neukamm & Richter (2025), explicit homogenized linear-elastic formulas/examples
  for isotropic laminates in a rigorous homogenization setting:
  https://doi.org/10.1007/s00526-025-03018-1
- Berdichevsky & Islam (2024), review of Hashin-Shtrikman bounds:
  https://doi.org/10.1016/j.ijengsci.2023.104015

### Internal state / thermodynamics

- Maugin (2015), review of internal variables in continuum thermomechanics:
  https://doi.org/10.1016/j.mechrescom.2015.06.009
- Collins & Houlsby (2000), thermomechanical constitutive framework with energy
  potentials and internal variables:
  https://doi.org/10.1016/S0749-6419(99)00073-X

---

## Explicitly deferred

M8A does not select or implement:

- full 3-D elasticity;
- constitutive tensor storage as material identity;
- finite-element or FFT homogenization;
- RVE generation backend;
- a universal RVE/scale-separation threshold;
- Hashin-Shtrikman implementation;
- phase-field evolution;
- CALPHAD;
- DFT/atomistic/MD backend;
- viscoelasticity/plasticity/damage/fracture;
- thermal/electrical/chemical transport compiler;
- mature distributed material field over organism geometry;
- production Simulation integration;
- structural mutation of material causes.

These are later reviewed scientific checkpoints.

---

## M8B implemented: 3-D elastic constitutive/homogenization reference

M8B is documented in [elasticity architecture](ELASTICITY_ARCHITECTURE.md) and
implements MAT-2: Mandel-Kelvin tensor primitives, positive-energy isotropic K/G
constituents, exact periodic-laminate 3-D homogenization, exact phase-local fields,
Hill-Mandel checks and Voigt/Reuss tensor bounds.

This remains a Material Compiler calibration. It does **not** solve organism-scale
structural equilibrium, gravity loading or arbitrary numerical RVEs.

The next mechanics milestone is M9A: a controlled quasi-static structural-mechanics
PCP that consumes compiled elasticity from the Material Compiler.
