# Elasticity architecture

## Status

M8B freezes the first mature three-dimensional elastic constitutive/homogenization
architecture and adds MAT-2, an exact periodic-laminate calibration.

This is still a **Material Compiler / constitutive-response** milestone.

It does **not** solve organism structural equilibrium, gravity loading, deformation
of whole organisms, contact, buckling, dynamics or failure.

The core separation is:

```text
developed material state
        |
        v
Material Compiler
        |
        v
constitutive operator C
        |
        v
future structural-mechanics PCP
        |
        v
stress/deformation/work/failure consequences on organism geometry
```

---

## Approved M8B decisions

### M8B-A1 — Constitutive layer, not structural solver

M8B ends at the compiled constitutive operator and analytical homogenization
reference. The future organism-scale mechanics solver is a separate PCP milestone.

### M8B-A2 — First 3-D calibration regime

The first benchmark is:
- quasi-static;
- isothermal;
- geometrically linear;
- energy-based;
- three-dimensional linear elasticity.

This is a calibration regime, not universal alien material behavior.

### M8B-A3 — Infinitesimal strain has a validity domain

M8B uses

```text
epsilon = sym(grad u)
```

as its benchmark kinematics.

Finite deformation, large rigid rotation and geometrically nonlinear structure must
promote to later mechanics rather than being silently interpreted through
infinitesimal strain.

No universal strain cutoff is frozen.

### M8B-A4 — Energy-based constitutive calibration

The benchmark elastic energy is

```text
psi = 1/2 epsilon : C : epsilon
```

with

```text
sigma = C : epsilon.
```

### M8B-A5 — Positive-energy, self-adjoint benchmark

MAT-2 requires its benchmark elasticity operator to be major-symmetric/self-adjoint
on symmetric strain space and positive definite.

This is a deliberate benchmark restriction. Mature active, dissipative or other
constitutive systems are not required to fit this exact class.

### M8B-A6 — Physical object is a fourth-order operator

The material response is conceptually

```text
C : Sym(3) -> Sym(3)
```

not a raw inherited array of coefficients.

### M8B-A7 — Mandel-Kelvin numerical representation

M8B uses the orthonormal Mandel-Kelvin representation

```text
[11,22,33,sqrt(2)23,sqrt(2)13,sqrt(2)12]
```

for symmetric second-order tensors.

This preserves tensor inner products and elastic energy under ordinary Euclidean
matrix/vector operations.

The 6x6 matrix is a numerical representation of the physical fourth-order tensor,
not the material's heritable identity.

### M8B-A8 — Isotropic benchmark constituents use K and G

MAT-2 benchmark phases are parameterized by positive bulk and shear moduli:

```text
K > 0
G > 0
```

with

```text
sigma = K tr(epsilon) I + 2G epsilon_dev
```

and

```text
psi = 1/2 K tr(epsilon)^2 + G epsilon_dev:epsilon_dev.
```

Young's modulus, Poisson ratio and Lame lambda are derived conveniences.

Positive K and G allow physically valid auxetic benchmark states.

### M8B-A9 — Anisotropy arises from structure

Material symmetry and anisotropy are consequences of developed structure and
orientation. MAT-2 starts from isotropic phases but produces anisotropic effective
response through layering.

### M8B-A10 — Rotation covariance

Rotating the material state rotates C covariantly.

Rotating state and loading together preserves elastic energy.

### M8B-A11 — MAT-2 exact periodic laminate

MAT-2 is a perfectly bonded periodic two-phase planar laminate.

Each phase is isotropic at the benchmark constituent level. The effective material
is generally transversely isotropic about the layer normal.

The effective tensor and local fields are obtained analytically, not through a
discretized RVE solve.

### M8B-A12 — Voigt/Reuss interpreted tensorially

MAT-2 records:
- Voigt stiffness upper-bound tensor;
- Reuss stiffness lower-bound tensor.

These are checked through elastic energy ordering.

They are not treated as universal scalar Young-modulus recipes.

### M8B-A13 — Hill-Mandel is executable

MAT-2 recovers exact phase-local strain/stress fields and verifies

```text
<sigma:epsilon> = <sigma>:<epsilon>
```

for the analytical laminate.

Equivalent macro/micro elastic energy is also checked.

### M8B-A14 — RVE boundary conditions remain distinct

Future finite-volume homogenization must preserve the distinction between:
- kinematic uniform boundary conditions;
- static/traction uniform boundary conditions;
- periodic boundary conditions.

They have different finite-sample apparent responses and must remain visible in
provenance/validity.

MAT-2 itself uses the exact periodic homogenized laminate and does not solve a
finite RVE boundary-value problem.

### M8B-A15 — No universal RVE threshold

M8B adds no rule such as

```text
macro_scale / micro_scale > N -> homogenized
```

for any universal N.

Finite-scale representativity remains QoI/state/statistics dependent.

### M8B-A16 — Bounds before expensive full-field physics

Tensor/energy bounds are legitimate cheap evidence. A future compiler may stop at
bounds when every admissible response leads to the same scientific/evolutionary
conclusion.

### M8B-A17 — Static local response is not dynamic response

MAT-2 does not certify wave propagation, frequency dependence, effective inertia,
Willis coupling, temporal memory or spatial nonlocality.

Static local elasticity must not be extrapolated into those regimes for
convenience.

### M8B-A18 — Full-field numerical method remains open

FEM, FFT, asymptotic homogenization and other numerical methods are U6
`numerical_realization` backends.

M8B does not make one of them architecture.

### M8B-A19 — Adversarial numerical regimes

Near-incompressibility and high phase contrast are mandatory future solver
challenge cases because numerical artifacts such as locking or poor conditioning
could create fake evolutionary advantage.

M8B includes analytical challenge fixtures but no FEM locking test because there
is no FEM solver yet.

### M8B-A20 — Nonlinear/failure physics remain later models

Plasticity, viscoelasticity, damage, fracture, finite strain, activity and dynamics
are outside MAT-2.

### M8B-A21 — Distinct uncertainty/error channels

Keep separate:
- material-state uncertainty;
- constitutive model-form uncertainty;
- homogenization/model-reduction uncertainty;
- numerical error.

### M8B-A22 — Ceff is not inherited

Evolution never mutates entries of the compiled effective elasticity tensor.

Evolution changes developed material state/construction; the compiler derives C.

---

## 3-D tensor foundation

### SymmetricTensor3

M8B represents physical stress/strain as symmetric second-order tensors with six
independent components.

Kelvin conversion is orthonormal:

```text
epsilon_K =
[epsilon11, epsilon22, epsilon33,
 sqrt(2)epsilon23, sqrt(2)epsilon13, sqrt(2)epsilon12]
```

so

```text
A:B = A_K dot B_K.
```

### Rotation3

A material orientation is represented with a proper orthonormal rotation.

The implementation validates:
- finite matrix entries;
- R^T R = I within numerical tolerance;
- det R = +1.

For a laminate layer normal, a robust local basis is built with local e3 aligned
to that normal.

### KelvinElasticityTensor

The numerical 6x6 Kelvin operator:
- is finite;
- is major-symmetric for the MAT-2 energy-based class;
- is positive definite;
- evaluates stress and elastic energy;
- rotates covariantly;
- is never exposed as an evolvable material-state primitive.

---

## Isotropic benchmark elasticity

For each phase:

```text
lambda = K - 2G/3
```

```text
E = 9KG/(3K+G)
```

```text
nu = (3K-2G)/(2(3K+G)).
```

Near-incompressibility appears physically as large K/G.

That is a physical regime.

Any future numerical locking is solver error and must not be "fixed" by changing
the material state.

---

## MAT-2 exact laminate homogenization

Choose local coordinates where the layer normal is e3.

Kelvin ordering:

```text
[11,22,33,sqrt(2)23,sqrt(2)13,sqrt(2)12]
```

Partition:

```text
T = [11,22,12] -> indices [0,1,5]
N = [33,23,13] -> indices [2,3,4].
```

For phase r:

```text
[s_T]   [C_TT C_TN] [e_T]
[s_N] = [C_NT C_NN] [e_N].
```

Perfect bonding and one-dimensional layering imply common tangential strain and
common normal traction.

Define

```text
A = sum_r f_r C_NN(r)^-1

B = sum_r f_r C_NN(r)^-1 C_NT(r)

D = sum_r f_r
    [C_TT(r) - C_TN(r) C_NN(r)^-1 C_NT(r)].
```

Then the exact local effective blocks are

```text
Ceff_NN = A^-1

Ceff_NT = A^-1 B

Ceff_TN = B^T A^-1

Ceff_TT = D + B^T A^-1 B.
```

The local tensor is then rotated into the global material orientation.

This calculation is analytical block algebra. MAT-2 introduces no mesh/voxel/RVE
discretization error.

---

## Exact local fields

Given a macro strain in local laminate coordinates:

```text
e_T = common tangential strain
E_N = macro normal-coupled strain.
```

The common normal traction is

```text
s_N = A^-1 (E_N + B e_T).
```

For phase r:

```text
e_N(r) =
C_NN(r)^-1 [s_N - C_NT(r)e_T].
```

This reconstructs exact phase-local strain/stress.

MAT-2 verifies:
- volume-average micro strain = macro strain;
- volume-average micro stress = macro stress;
- average micro work = macro work;
- average micro elastic energy = macro elastic energy.

---

## Voigt and Reuss tensor bounds

MAT-2 provides

```text
C_V = sum_r f_r C_r
```

and

```text
C_R = [sum_r f_r C_r^-1]^-1.
```

For representative strain states, the exact effective energy is required to lie
between the Reuss and Voigt tensor-energy bounds.

This is deliberately a tensor/energy statement.

---

## MAT-1 compatibility

MAT-1 used scalar one-dimensional constituent axial moduli.

MAT-2 uses full 3-D isotropic K/G phases, so naive comparisons generally represent
different boundary-value problems because of lateral/Poisson coupling.

A genuine overlap exists when each isotropic phase has zero Poisson ratio
(lambda=0):

```text
G = E/2
K = E/3.
```

Then normal strain components decouple and MAT-2 reproduces MAT-1's exact scalar:
- in-plane arithmetic response;
- layer-normal harmonic response.

This is the only compatibility comparison currently required.

---

## ACP validity

MAT-2's compiled response explicitly declares:
- quasi-static;
- isothermal;
- infinitesimal strain;
- linear energy-based elasticity;
- perfect interfaces;
- periodic homogenized laminate;
- no finite-scale RVE certification;
- no dynamic/nonlocal certification;
- no damage/plasticity/viscoelasticity/activity.

The compiled tensor is therefore a certified response only inside that regime.

---

## Scientific update points

MAT-2 uses M8A update points:

```text
constituent_physics
homogenization
constitutive_response
numerical_realization
calibration_reference
```

It does not pretend to use:
- phase_thermodynamics;
- microstructure_evolution.

Those remain separate future science.

---

## Literature basis

- Pindera et al. (2012), exact effective properties and local fields of periodic
  layered composites:
  https://doi.org/10.1016/j.mechrescom.2012.08.007
- Ostoja-Starzewski (2006), scale-dependent SVE/RVE concepts and bounds:
  https://doi.org/10.1016/j.probengmech.2005.07.007
- Nagel et al. (2016), Kelvin mapping and tensor-consistent finite-element
  implementation:
  https://doi.org/10.1007/s12665-016-5429-4
- Computational-homogenization literature on Hill-Mandel and KUBC/SUBC/PBC:
  https://www.sciencedirect.com/science/article/pii/S0045782519304281

---

## Explicitly deferred

M8B does not implement:
- organism structural equilibrium;
- gravity/body-force mechanics;
- FEM/FFT microstructure solver;
- arbitrary/random microstructure RVE;
- universal RVE threshold;
- finite deformation;
- inertial/dynamic mechanics;
- nonlocal/gradient mechanics;
- plasticity/viscoelasticity/damage/fracture;
- active stress;
- contact/buckling;
- production development/material-field wiring.

---

## Next mechanics milestone

The natural next milestone is **M9A: quasi-static structural mechanics PCP**.

M9A should consume compiled local elasticity from the Material Compiler and solve
organism-scale balance laws such as

```text
div sigma + rho b = 0
```

on a controlled reference geometry.

That is the point where gravity can first act through actual body force -> stress ->
deformation/work rather than a shortcut morphology rule.

Full-field material homogenization (future M8C) should be added when M9A or evolved
material states create a real need for numerical microstructure solves, rather than
preemptively selecting FEM/FFT now.
