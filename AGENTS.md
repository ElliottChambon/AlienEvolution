# Agent instructions

## Scientific scope

- Treat AlienEvolution as a scientific computational framework, not a visual
  alien generator. Follow [the mature architecture](docs/MATURE_ARCHITECTURE.md),
  [scientific debt](docs/SCIENTIFIC_DEBT.md), [roadmap](docs/ROADMAP.md), and
  [benchmarks](docs/BENCHMARKS.md).
- Preserve the causal architecture: environment/physics -> development ->
  phenotype -> physical/energetic consequences -> selection/evolution.
- Avoid hard-coded Earth anatomy and direct environment-to-trait lookup rules.
  Use Earth biology as evidence and validation data, not a mandatory alien
  body-plan template.
- Prefer mechanisms grounded in physics, chemistry, energetics, information,
  and transport; make uncertainty explicit. Keep scientific state separate
  from visualization.
- Record simplifications in `docs/SCIENTIFIC_DEBT.md` rather than silently
  turning them into permanent assumptions. Before freezing a major biological
  or physical mechanism, inspect relevant project docs and note where external
  literature review is still required.

## Implementation and validation

- Follow approved S1–S5 in [sensing foundations](docs/SENSING_FOUNDATIONS.md),
  [physical coupling](docs/PHYSICAL_COUPLING_ARCHITECTURE.md), and
  [heritable construction](docs/HERITABLE_CONSTRUCTION_ARCHITECTURE.md).
  Preserve `SensoryProgram` and B4–B6 as prototype history/compatibility, not
  the mature endpoint. Do not introduce fixed named-sense catalogs or primitive
  semantic channels. Mature senses are diagnosed causal/informational PCP pathways.
- Mature evolution changes Heritable Construction State through physically
  justified inheritance events and explicit mutation footprints. Do not use
  arbitrary signal/target-ID retargeting, software-graph-size mutation hazards,
  automatic edge cloning, target repair, or permanent biological role labels.
  Direct effective-parameter mutation requires explicit reduced-experiment debt.
- Keep provenance DAG, derived dependency graph, compilers/caches, and scientific/
  numerical solver choice simulator-owned; they are never inherited biology.
  Evolution changes `phi`; the simulator controls `m`.
- Follow [material architecture](docs/MATERIAL_ARCHITECTURE.md): mature material
  identity is scale-aware physical state, not an inherited/evolved property sheet.
  Effective stiffness, conductivity, diffusivity, permeability, strength, etc. are
  compiled responses with query/QoI validity and provenance. Do not silently
  homogenize when representativity is unresolved, and do not promote MAT-1's scalar
  arithmetic/harmonic laminate calibration into a general 3-D material law.
- Follow [elasticity architecture](docs/ELASTICITY_ARCHITECTURE.md): MAT-2's
  3-D elasticity tensor and exact laminate homogenization remain Material Compiler
  reference physics. Do not treat compiled C as inherited state, do not replace
  Mandel-Kelvin/tensor energy with ad hoc scalar stiffness, and do not claim that
  MAT-2 solves organism structural equilibrium or certifies arbitrary finite RVEs.
- Reductions require quantity-specific validity/error/reference/provenance/version
  records and selection-aware adversarial checks. Separate numerical uncertainty
  from scientific/model-form uncertainty; pruning cannot erase future possibilities.
- Treat computational cost as a first-class simulator constraint without trading
  away scientific adequacy. Keep high-fidelity reference models, runtime physical
  models, and compiled/reduced models as distinct roles; use the cheapest model
  already adequate for the current QoI, amortize expensive references through
  validated caching/sparse audits where appropriate, and never make solver/cost
  policy inherited or evolvable organism state.
- Preserve evidence categories and unresolved alternative mechanism models.
  Stop and report ambiguities requiring new scientific architecture decisions;
  specific mechanism/inheritance backends require explicit scientific checkpoints.

- Use C++20 and the existing CMake structure. Update CMake deliberately; do not
  add test sources to unrelated executables.
- Remove obsolete production code after validating its replacement instead of
  retaining dead implementations.
- Preserve deterministic reproducibility where expected. Add or update tests
  with every new subsystem.
- Build in Release mode and run the full fast CTest suite before considering a
  coding task complete. Typical commands from the repository root are:

  ```sh
  cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  ctest --test-dir build -C Release --output-on-failure
  ```

- Long evolutionary validation experiments are milestone/manual validation
  runs; do not run them on every change.

## Delivery

- Make small, reviewable feature branches and open PRs into `main`, subject to
  the user's requested delivery scope. Do not merge automatically unless
  explicitly instructed.
- In PR descriptions, summarize architecture, scientific assumptions/debt,
  tests run, and unresolved limitations.
