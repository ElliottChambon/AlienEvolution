# Chemical coupling architecture and CHEM-1

This records the approved M7/CHEM-1 decisions in
[issue #35](https://github.com/ElliottChambon/AlienEvolution/issues/35), specializing
[S1–S5](PHYSICAL_COUPLING_ARCHITECTURE.md), [sensing foundations](SENSING_FOUNDATIONS.md)
and [Heritable Construction State](HERITABLE_CONSTRUCTION_ARCHITECTURE.md).
M7A implements the analytical and well-mixed scaffold only. Mature chemical sensing
and the full spatial CHEM-1 benchmark are incomplete.

## Causal chain and physical support

The mature chain is environment/material chemical composition and transport
-> physical access at coupling support -> association/reaction and material
state changes -> downstream physical/developmental consequences -> energetic
and functional consequences -> selection. Senses are diagnosed causal/informational
pathways, not primitive named chemical channels.

Physical coupling support is the material region/interface and contact geometry
where access and interaction occur. Accessibility, transport, geometry and material
composition must cause the interaction. Software ports, semantic ligand roles and
signal IDs cannot create access or biological function. The CHEM-1 sphere is an
ideal benchmark geometry, not anatomy. `chem1.ideal_spherical_support` is an inert
schema association, not an implemented geometry framework or runtime connection.

Chemical amount/composition and thermodynamic activity are distinct. Constituent
counts belong to matter accounting; activity reflects chemical potential under
declared thermodynamics. Concentration here is an explicitly ideal-dilute input,
not a universal substitute for activity. Consistent number-based units are `a`
in m, `D` in m²/s, `k_a`/`k_on` in m³/s, `k_d`/`k_off` in 1/s, and `c`/`K_D` in
1/m³. No unit conversion engine is supplied.

Mature coupling requires conservative, bidirectional exchanges: association removes
free constituent and changes reactive material state; dissociation returns it.
Matter/energy bookkeeping and environmental feedback cannot be replaced with
one-way signal copying. An externally maintained reservoir is a declared open-system
reduction. M7A records probability balance only; it does not implement environmental
matter accounting or explicit energy exchange.

## State, reactions, thermodynamics and compiled transitions

Interactions are processes changing physical material state. State/reaction
descriptions must retain the degrees of freedom needed by their QoIs. Markov
models are scoped reductions: discarding spatial rebinding/history does not make
the underlying physics memoryless.

Passive reversible chemistry must respect equilibrium/detailed balance under its
declared thermodynamics. Driven chemistry needs explicit driving and energetic
accounting; independently adjusting effective forward/reverse rates cannot replace
that accounting. CHEM-1A is passive and satisfies
`k_on*c*(1-b_eq) = k_off*b_eq` where defined.

Elementary parameters and compiled effective transitions are distinct. `k_a` and
`k_d` are intrinsic benchmark inputs; `k_on`, `k_off`, rebinding and `K_D` are
compiled ideal-sphere quantities, not new inherited traits or mutation targets.
Material changes must cause altered chemistry through a justified mechanism.
Arbitrary signal/target IDs and permanent biological role labels cannot substitute
for material causality. There is no HCS, reproduction or mutation integration.

## Frozen CHEM-1 problem

`L + S <-> LS` denotes mobile constituent L, generic reactive material state/site
S and associated state LS. No food/toxin roles, receptor classes or smell/taste
ontology are assigned. The canonical mature reference is ideal 3D diffusion with
relative diffusion coefficient `D` to a sphere of radius `a`, finite intrinsic
reactivity `k_a` and intrinsic dissociation `k_d`:

```text
k_D = 4*pi*D*a
chi = k_a/k_D
k_on = k_D*k_a/(k_D+k_a)
p_rebind = k_a/(k_a+k_D)
k_off = k_d*k_D/(k_a+k_D)
K_D = k_d/k_a = k_off/k_on
```

`chi` measures reaction versus diffusion limitation, not fidelity. Small `chi`
approaches `k_on=k_a`; large `chi` approaches `k_on=k_D`; `chi=1` gives `k_D/2`.
These ideal-dilute spherical relations and collapsed rebinding do not describe
arbitrary geometry, dense mixtures or cooperative chemistry. `DiffusiveField2D`
is neither the reference nor a source of literal 3D rates.

The effective Markov rates and collapsed rebinding probability are the intended
reduction when rapid local rebinding can be integrated out and rebinding interference
is negligible. Ideal spherical geometry alone does not guarantee this condition:
spatial history and many-particle interference can invalidate the two-state Markov
description even within ideal-dilute chemistry. This is a declared validity
assumption, not an evaluated criterion or a newly selected timescale threshold.

For an open, well-mixed constant reservoir and one reactive site:

```text
0 --k_on*c--> 1
1 --k_off----> 0
b_eq = c/(K_D+c)
tau_R = 1/(k_on*c+k_off)
db/dt = k_on*c*(1-b)-k_off*b
b(t) = b_eq+(b0-b_eq)*exp[-(k_on*c+k_off)*t]
```

Single-site occupancy is noncooperative; no free Hill coefficient is fitted.
Equal `K_D` can coexist with different response times. Constant-concentration
segments can compose imposed transient forcing; this does not solve reservoir
transport or a time-varying reaction network.

For `k_d>0`, `c=0` gives `b_eq=0`. At the permitted boundary `k_d=0,c=0`, both
hazards vanish and every initial occupancy is stationary. The equilibrium helper
reports `std::domain_error` for this nonunique equilibrium; relaxation is infinite
and deterministic/CTMC dynamics preserve initial state. For `k_d=0,c>0`, binding
is absorbing and equilibrium occupancy is one. These follow the frozen equations.

The frozen canonical spatial benchmark has a reactive sphere at radius `a`.
Its **open/reservoir** configuration maintains external concentration for
association-rate validation. Its **closed finite spherical domain** has a
reflecting outer boundary for depletion/conservation tests. These approved spatial
configurations are non-executable declarations and remain unimplemented in M7A;
the well-mixed reservoir helper does not implement either spatial domain.

The closed configuration must preserve free L plus bound L
and unbound S plus LS. Depletion changes concentration and couples chemistry to
transport and matter accounting. That configuration is a declared challenge,
not executable M7A physics.

## Multidimensional ACP and M1–M3 integration

Representations have capability profiles, not a universal scalar ranking:

| Representation | Spatial | Stochastic | Kinetic/history |
| --- | --- | --- | --- |
| Analytical relations | Ideal sphere compiled into rates | No trajectories | Rate relations only |
| Equilibrium reduction | Collapsed | Mean equilibrium | Discarded |
| Deterministic well-mixed | Collapsed | Mean occupancy | Constant-reservoir Markov relaxation |
| Well-mixed CTMC | Collapsed | Intrinsic binary jumps | Constant-reservoir Markov trajectory |
| Future spatial reference | Explicit support/transport; method unresolved | Spatial stochastic | Rebinding history to be resolved |

Implemented profiles share ideal-dilute thermodynamics, one reversible pair as
chemical detail and no driving. Later models can vary these axes independently.
Numerical refinement within a model is distinct from physical model promotion.
Intrinsic physical fluctuations and sampling uncertainty are distinct from
numerical error and scientific/model-form uncertainty.

M1 provides `alien_evolution.chem1.*` identities/version `chem1a-v1`, physical
inputs, assumptions, provenance and reviewed literature context. P1 physical
interaction evidence does not establish biological sensory use, calibration or
spatial validation. M2 provides model-owned concentration/state/observables and
probability ledger vocabulary for kinetics. Algebraic profiles have no dynamic
state; CTMC state is binary and deterministic state is mean bound probability.
There is no universal chemical quantity enum, runtime port wiring or solver API.

M3 declares QoI-specific validity/reference/error vocabulary: rates, `chi`,
rebinding, `K_D`, equilibrium occupancy where applicable, transient response and
relaxation time for kinetics, stochastic mean/variance for CTMC. M7B now implements
`alien_evolution.chem1.spatial_reference` version `chem1b-v1` for the exact
isolated-pair radial **density QoIs only**. The existing M7A contracts retain their
`review-pending` spatial-reference edges because M7B does not yet provide the
overlapping reservoir occupancy/rate QoIs needed to certify those reductions.
Analytical checks apply only to declared QoIs; the existence of a spatial model
does not resolve omitted physics automatically. Contracts do not evaluate criteria,
calculate errors or select/promote models. Numerical differences do not bound
model-form uncertainty or constitute biological validation.

## Implemented subset and numerical verification

[ReversibleChemicalAssociation](../include/alien_evolution/physics/ReversibleChemicalAssociation.hpp)
implements validated physical parameters, analytical rates, equilibrium, derivative,
exact constant-concentration response and two-state CTMC using caller `ae::Random`.

Inputs are finite with `a,D,k_a>0`, `k_d,c,time>=0` and occupancy in `[0,1]`.
There are no silent input clamps. Scaled products and ratio algebra avoid simple
intermediate overflow; independently calculated probability complements avoid
subtracting nearly one; `expm1` preserves short-time response. Positive derived
rates/ratios outside finite-double range, including underflow to zero, throw
`std::overflow_error`. Finite valid physical inputs do not ensure derived quantities
are representable. Probability ratios can round to zero/one at extreme separation.
Relaxation time can overflow to infinity or underflow to zero at double limits.
An overflowing decay exponent returns the rounded long-time limit. No physical
threshold or equation is changed to repair these cases.

CTMC waits are exponential. Each proposed wait consumes a nonzero uniform draw
(zero is retried); the terminal censored wait consumes its draw. Zero duration
and absorbing states consume none. Events alternate with strictly increasing
times within the horizon. Unrepresentable increments throw; the explicit event
safety limit throws instead of silently truncating. Reproducibility follows the
existing RNG/platform numerical conventions, with no hidden stream or production
RNG changes.

[Fast tests](../tests/reversible_chemical_association_tests.cpp) check input rejection,
chi sweep/limits/crossover, rebinding/effective-rate/K_D consistency, probability
bounds, equilibrium flux balance, exact response/segment composition, identical
affinity with different kinetics and degenerate boundaries. CTMC tests check
seeded trajectories, alternating states, monotonic time, absorbing/no-draw cases
and stationary mean/variance. The predeclared ensemble test uses 12,000 trajectories,
seed 3502, 16 relaxation times and absolute mean/Bernoulli-variance tolerance 0.025.
These are analytical/well-mixed checks, not spatial or biological validation.

## M7B exact isolated-pair spatial reference

[ReversibleChemicalAssociationSpatialReference](../include/alien_evolution/physics/ReversibleChemicalAssociationSpatialReference.hpp)
implements the first reviewed spatial reference for CHEM-1. It evaluates the exact
3-D radial Green's function for one isolated pair under the
Smoluchowski/Collins-Kimball contact-reactivity model with reversible back-reaction,
using the published Kim-Shin solution as reproduced by Prüstel and
Meier-Schellersheim (2021), Appendix A, Eqs. A12-A13.

The reference nondimensionalizes the problem with
`chi=k_a/(4*pi*D*a)`, `delta=k_d*a^2/D`, `rho=r/a`,
`rho0=r0/a`, and `tau=D*t/a^2`. Dimensionless characteristic roots solve
`lambda^3-(1+chi)lambda^2+delta*lambda-delta=0` and are cached in the
reference object. Ordinary reversible regimes may contain a complex-conjugate
root pair; the final physical density is real. The exact `k_d=0` radiation-boundary
limit is evaluated separately rather than forcing the repeated-root A12 expression.

The complex scaled complementary error function required by A12 is numerical
infrastructure, not scientific architecture. The implementation evaluates a stable
right-half-plane integral for moderate arguments and the standard asymptotic
expansion for large arguments, with independent high-precision golden fixtures.
Near-degenerate characteristic roots are rejected by an explicit numerical
conditioning policy rather than silently amplifying cancellation. Numerical
special-function/root error remains distinct from the scientific/model-form
uncertainty of choosing the SCK contact-reactivity model itself.

M7B currently exposes volume probability density and radial-shell probability
density. It does **not** yet expose a verified direct bound-probability,
time-dependent association-rate, maintained-reservoir occupancy, or finite-bath
matter-conservation QoI. Therefore it is a spatial reference kernel/oracle, not
full certification of the M7A CTMC or equilibrium reduction.

The isolated-pair configuration is physically distinct from M7A's maintained
constant-concentration reservoir. In unbounded 3-D, an isolated reversible pair
can ultimately escape; it must not be required to approach Langmuir equilibrium.
Finite/open baths, depletion, conservation and many-particle arrival statistics
remain M7C+ work.

## M7C conserved competitive finite-bath reference

[ReversibleChemicalAssociationFiniteBath](../include/alien_evolution/physics/ReversibleChemicalAssociationFiniteBath.hpp)
implements the next CHEM-1 calibration layer: a fixed total number of identical
ligands diffuse in a concentric spherical shell and compete for one capacity-one
reactive target. The outer sphere is reflecting. When the site is free, the inner
sphere has the Collins-Kimball Robin condition; while occupied, competitors see a
reflecting inner boundary. Dissociation returns the same bound ligand to `r=a`
without an arbitrary unbinding radius.

Spherical symmetry is used aggressively for computational control: each free ligand
is represented only by its radial coordinate. The radial diffusion equation is
reduced with `u=r p` to a one-dimensional heat equation. Robin-Neumann and
Neumann-Neumann propagators are represented by cached spectral modes whose
dimensionless roots obey the pole-free phase equation

```text
z*(Lambda-1) - atan(H_in/z) + atan((1/Lambda)/z) = n*pi
```

with `H_in=1+chi` for a free/reactive site and `H_in=1` for an occupied/
reflecting site. The reflecting kernel additionally includes the uniform zero mode.

The competitive trajectory is event-driven rather than Brownian-timestep based.
When the site is free, the earliest binding time is sampled from the product of
single-ligand survival probabilities. The winner is selected by its instantaneous
first-reaction hazard, while all nonwinning ligands are propagated from the
**conditional distribution given that they did not react**. While occupied,
competitors propagate with the reflecting kernel until intrinsic dissociation.
Thus the ligands are independent in bulk motion but coupled through target
availability; M7C is not `N` independent M7B copies.

M7C adds an actual matter-count invariant for this calibration problem:

```text
N_free + B = N_total,   B in {0,1}
```

and exact analytical anchors including the bounded mean first-reaction time and

```text
P_bound(eq) = N / (N + K_D V).
```

For this ideal single-site finite system, writing total number concentration as
`c_total=N/V` makes the equilibrium expression algebraically identical to
`c_total/(c_total+K_D)`; finite-bath physics is therefore diagnosed primarily
through kinetics, depletion, confinement, low-copy noise, rebinding/history and
correlations rather than by inventing a different equilibrium affinity curve.
A maintained open reservoir is still a distinct dynamical configuration.

### M7C numerical-resolution policy

The bounded spectral series becomes expensive at extremely short dimensionless
times. M7C-v1 therefore declares `minimumResolvedDimensionlessTime`,
`spectralTolerance`, and `maxModes` as simulator-owned numerical settings.
Mode tables are compiled once per physical configuration and reused. A propagation
below the declared time floor, or a requested tolerance that cannot be met within
the mode budget, is reported as numerically unresolved; the implementation does
not silently switch to Brownian dynamics, loosen accuracy, or modify organismal
physics.

The attenuation-based mode-count rule is a numerical policy, not a universal error
bound. M7C-R1 keeps the same compiled table and physical model but evaluates only
the ordered **active prefix** needed by that attenuation policy at the actual query
time. At `tau=tau_min` the active prefix equals the compiled table; at longer
times it can shrink toward the minimum numerical mode count. This is simulator-owned
cost adaptation only: it does not change trajectories, chemistry, the time-resolution
boundary or scientific validity.

Fast tests use independent high-precision fixtures and exact analytical relations;
broader `N,chi,delta,Lambda` sweeps live in a manual experiment outside CTest.

A literature checkpoint considered a short-time M7B / long-time M7C hybrid.
An arbitrary clock switch was rejected because M7B is unbounded and M7C has a
reflecting outer boundary; such a switch would change the physical model without a
controlled handoff error. Two scientifically credible future routes remain:

1. **protective-domain first-passage coupling** in the GFRD/eGFRD sense, where
   local-domain exits/reactions are exact stochastic events before rebuilding the
   global decomposition; or
2. a verified **short-time Robin/multiple-reflection heat-kernel representation**
   of the same bounded problem, cross-validated in an overlap region against the
   current long-time spectral representation.

Neither route is implemented by M7C-R1. Either must carry explicit overlap/error
tests before it may remove the current `tau_min` boundary. The protective-domain
decision is grounded in [van Zon & ten Wolde (2005)](https://doi.org/10.1103/PhysRevLett.94.128103)
and the exact asynchronous eGFRD construction discussed by
[Takahashi, Tănase-Nicola & ten Wolde (2010)](https://doi.org/10.1073/pnas.0906885107).
The short-time alternative is supported by explicit/local Robin heat-kernel theory
and multiple-reflection constructions, e.g.
[Nursultanov, Rowlett & Sher (2024)](https://doi.org/10.1007/s40316-024-00237-4)
and [Bordag et al. (2002)](https://doi.org/10.1103/PhysRevD.65.064032).

M7C remains calibration geometry and a reference role. It does not make concentric
spheres, radial state, SCK contact reactivity, or the spectral algorithm into the
mature universal chemistry architecture.

## Computational role separation

Mature Adaptive Certified Physics distinguishes three roles:

```text
high-fidelity reference model
!= runtime physical model
!= compiled/reduced model
```

Reference models may be comparatively expensive and are intended for calibration,
challenge cases, validity-boundary checks, novelty/elite/sudden-gain audits and
random shadow evaluations. Routine evolution should normally use the cheapest
representation that is already scientifically adequate for the current QoI.
Scientific adequacy is a hard constraint; computational cost chooses only among
adequate alternatives. Model/cost choice is simulator-owned and never inherited.

M7B is deliberately designed for amortization: dimensionless roots depend only on
`(chi,delta)`, no Brownian timestep or spatial mesh is advanced, and broad validity
maps belong to manual milestone experiments rather than CI. Future caches,
interpolation tables or compiled response kernels must remain simulator-owned and
carry certified interpolation/reduction error.

## M7A simplifications and mature replacements

M7A is deliberately a scaffold. The following simplifications are **not** intended
to become the permanent chemical-sensing architecture:

| M7A simplification | Mature replacement / addition |
| --- | --- |
| Ideal-dilute scalar concentration | Thermodynamic closure from conserved composition/state to activity/chemical potential, including nonideal and electrochemical models where required |
| Spherical support and constant open reservoir | General physical coupling support over surfaces, volumes, porous/distributed regions and internal interfaces, with explicit open/closed domains and transport |
| Effective two-state Markov rates with collapsed rapid rebinding | M7B exact isolated-pair reference plus M7C conserved competitive finite-bath reference; later general/open-bath references remain required when evolved geometry, mixtures or broader many-particle physics matter |
| One ligand + one noncooperative site | Multispecies mixtures, competition/promiscuity, interacting sites/cooperativity and general physical state/reaction networks when justified |
| Passive reversible association only | Chemical transformation, adsorption/permeation, protonation, redox, catalysis and other mechanism families; driven cycles require explicit free-energy reservoirs |
| Probability normalization ledger only | Bidirectional matter, charge and energy/free-energy exchange accounting across the coupling support |
| Fixed benchmark `a,D,k_a,k_d` inputs | Developed material/geometry plus reviewed material/chemistry models compile or constrain effective rates and affinities; mature evolution does not directly mutate finished response constants |
| Declarative ACP contracts/challenges only | Runtime quantity-specific certification, promotion/demotion and evolutionary adversarial checks after reference models and error criteria are validated |
| Standalone benchmark, no production wiring | Eventual validated world/transport -> coupling support -> material-state change -> transduction/integration -> consequence path, followed by retirement of obsolete legacy sensing only after equivalence/coverage is demonstrated |

These replacements are requirements or reviewed future directions, not permission
to select their numerical methods without a new scientific checkpoint. The detailed
open items remain tracked in [scientific debt](SCIENTIFIC_DEBT.md#chem-1a-analytical-and-well-mixed-scaffold).

## Challenges, expected failures and deferred checkpoint

The M3 registry declares eight spatial comparisons, all **UNRESOLVED**:
reaction-limited, diffusion-limited, transient forcing, low-copy stochastic,
spatial rebinding, finite-domain depletion/conservation, spherical encounter-radius/
support-size variation (association-rate QoI), and near-validity-boundary
cross-fidelity disagreement at the onset of rebinding interference or spatial
memory within ideal-dilute chemistry. The boundary case concerns spatial/kinetic
fidelity, not dilute-to-nondilute thermodynamic promotion: the future ideal-dilute
spatial reference cannot certify nonideal thermodynamics. Nonspherical supports
remain a later extension, separate from the first geometry challenge.
Contexts are inert scenario
descriptions, not reproducible spatial snapshots before solver/configuration
review. Analytical passes do not mark spatial entries passed.

Expected failures include non-Markov rebinding/arrival correlations, closed-domain
depletion, altered support geometry, crowding/nonideal activity, mixtures and
competition, cooperative sites, explicit driving, amplification, adaptation and
proofreading. Mean occupancy cannot reproduce intrinsic fluctuations; equilibrium
cannot reproduce kinetic history. Numerical refinement alone cannot repair these
physical model limitations.

M7B selects the exact reversible isolated-pair Green's function as the first
CHEM-1 spatial **reference oracle**. It does not select a universal spatial solver.
Brownian dynamics, full GFRD/eGFRD, Smoldyn-style algorithms, RDME, general 3-D PDE
and many-particle spatial algorithms remain unchosen for their later runtime/reference
roles. Doi volume reactivity remains an alternative microscopic physical model,
not a higher-fidelity implementation of the same SCK model. Relevant context includes
[Kim & Shin (1999)](https://doi.org/10.1103/PhysRevLett.82.1578),
[Prüstel & Meier-Schellersheim (2021)](https://doi.org/10.1063/5.0037266),
[Andrews & Bray (2004)](https://doi.org/10.1088/1478-3967/1/3/001), and
[van Zon & ten Wolde (2005)](https://doi.org/10.1103/PhysRevLett.94.128103).

Mature sensing, nonideal thermodynamics, mixtures, electrochemistry/redox/protonation,
cooperativity, amplification/adaptation/proofreading, and evolutionary/selection-aware
cross-validation remain [debt](SCIENTIFIC_DEBT.md#chem-1a-analytical-and-well-mixed-scaffold).
Production `Simulation`, `SensoryProgram`, `RegulatoryInputInterface`, development,
mutation/reproduction, B4–B6 and the separate M6 phenomenological PCC benchmark
remain unchanged. M7A–M7C now provide a well-mixed scaffold, an exact isolated-pair
reference, and a conserved competitive finite-bath calibration reference. This is
still not full CHEM-1 or mature sensing: explicit open-reservoir dynamics, arbitrary
developed geometry, mixtures, nonideal chemistry and production migration remain
future reviewed work.
