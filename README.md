# pmzf

A CLI application for searching PubMed.

## Build

Requirements:

- C++20 compiler
- CMake 3.20 or newer
- libcurl development package

By default, CMake uses `reference/libspagyrist` as the Spagyrist source tree.
Use `PMZF_SPAGYRIST_PROVIDER=fetch` or `package` when using a remote or
installed Spagyrist instead.

```sh
mkdir -p build
cd build
cmake ..
make
```

Run tests:

```sh
ctest --output-on-failure
```

## Usage

```sh
./build/pmzf "cancer"
./build/pmzf -s fzf "infundibular basal keratinocytes"
./build/pmzf -f markdown "CRISPR"
./build/pmzf --date-range "2024:2025[DP]" "COVID-19"
./build/pmzf --retmax 5 "single cell RNA sequencing"
```

After a candidate is selected, pmzf fetches PubMed details and displays the
PubMed URL, bibliographic metadata, and abstract text when PubMed provides one.

## Spagyrist Options

Common options connected to Spagyrist selector, renderer, and output behavior.

- `-f, --format <terminal|markdown|plain>`: output format. Default: `terminal`.
- `-o, --output <stdout|editor>`: output target. Default: `stdout`.
- `-s, --select <builtin|fzf|number>`: candidate selector. Default: `builtin`.
  If `builtin` is not available, pmzf falls back to the number selector.
- `--version`: show version information.
- `--info`: show Spagyrist runtime information.

## pmzf Options

Options for PubMed searching and E-utilities access.

- `-d, --date-range <range>`: PubMed date range filter.
  Example: `2024:2025[DP]`.
- `-r, --retmax <n>`: maximum results to fetch. Default: `20`.
- `--limit <n>`: compatibility alias for `--retmax`.

### Environment Options

- `PMZF_BASE_URL`: E-utilities base URL.
  Default: `https://eutils.ncbi.nlm.nih.gov/entrez/eutils`.
- `PMZF_TOOL_NAME`: tool name sent to NCBI. Default: `pmzf`.
- `PMZF_EMAIL_ADDR`: email address sent to NCBI. Default: `me@example.com`.
- `NCBI_API_KEY`: optional NCBI API key.

## License

License information has not been added yet.

Third-party dependency licenses and source information are listed in
`THIRD_PARTY_NOTICES.md`.
