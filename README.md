# pmzf

`pmzf` is a PubMed search client built around `libspagyrist`.

This repository is being structured after `reference/wzf`, while the PubMed-specific behavior is being migrated from `reference/PubMed-Search`.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

By default, CMake uses `reference/libspagyrist` as the Spagyrist source tree.
