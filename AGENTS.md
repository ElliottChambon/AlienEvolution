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
