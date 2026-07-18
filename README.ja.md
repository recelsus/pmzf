# pmzf

`pmzf` は `libspagyrist` に対応する PubMed 検索クライアントです。

構成は `reference/wzf` を踏襲し、PubMed 固有の処理は `reference/PubMed-Search` から移植していく前提です。

## ビルド

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

既定では `reference/libspagyrist` を Spagyrist のソースツリーとして使います。
