# pmzf

CLIでPubMedを検索するアプリケーション。

## Build

必要なもの:

- C++20 対応 compiler
- CMake 3.20 以上
- libcurl development package

デフォルトでは `reference/libspagyrist` を Spagyrist のソースツリーとして使います。
remote または installed Spagyrist を使う場合は `PMZF_SPAGYRIST_PROVIDER=fetch`
または `package` を指定します。

```sh
mkdir -p build
cd build
cmake ..
make
```

テストを実行する場合:

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

候補を選択すると、PubMed details を取得し、PubMed URL、bibliographic metadata、
PubMed が提供する場合は abstract text を表示します。

## Spagyrist Options

Spagyrist の selector / renderer / output に繋がる共通的なオプションです。

- `-f, --format <terminal|markdown|plain>`: 出力形式。デフォルトは `terminal`。
- `-o, --output <stdout|editor>`: 出力先。デフォルトは `stdout`。
- `-s, --select <builtin|fzf|number>`: 候補選択方法。デフォルトは `builtin`。
  `builtin` が利用できない場合はnumber selectorにfallback します。
- `--version`: versionを表示。
- `--info`: Spagyristの実行環境情報を表示。

## pmzf Options

PubMed 検索と E-utilities access に関わるオプションです。

- `-d, --date-range <range>`: PubMed の date range filter。
  例: `2024:2025[DP]`。
- `-r, --retmax <n>`: 取得する検索候補数。デフォルトは `20`。
- `--limit <n>`: `--retmax` の互換 alias。

### Environment Options

- `PMZF_BASE_URL`: E-utilities base URL。
  デフォルトは `https://eutils.ncbi.nlm.nih.gov/entrez/eutils`。
- `PMZF_TOOL_NAME`: NCBI に送信する tool name。デフォルトは `pmzf`。
- `PMZF_EMAIL_ADDR`: NCBI に送信する email address。デフォルトは `me@example.com`。
- `NCBI_API_KEY`: optional NCBI API key。

## License

pmzf is licensed under the MIT License. See `LICENSE` for details.

Third-party dependency licenses and source information are listed in
`THIRD_PARTY_NOTICES.md`.
