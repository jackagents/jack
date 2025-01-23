# C-BDI Make

## Requirements

| Dependency | Vendored | License | Distribute License | Debian/Ubuntu apt | Notes                                                                                                                                    |
| ---------- | -------- | ------- | ------------------ | ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| rust       | No       |         |                    |                   | Build cbdi-make and cbdi validation library, see https://rustup.rs for installation                                                      |
| wasm-pack  | No       |         |                    |                   | **(Optional)** Build cbdi validation library as a javascript webassembly library depends on rust install-- use "cargo install wasm-pack" |

### CBDI-Make

The `cbdi-make` is a sub-project of `tools/cbdi-validator` vendored into the code library for code generating CBDI stubs from a CBDI-model.

For debug and release respectively run the following commands in the `tools/cbdi-validator` directory.

```bash
cargo build
cargo build --release
```

The binaries are available in the following location, `tools/cbdi-validator/[debug|release]/cbdi-make.exe`.

To create a `.deb` package

1. Install the pre-requisite package via `cargo install cargo-deb`
2. Run `cargo deb` in the `tools/cbdi-validator` directory

### CBDI-Validator WASM

Like CBDI-Make, the validator code is part of the same subproject in `tools/cbdi-validator`.

To build the library run the folowing commands in the root directory:

```bash
wasm-pack build --target web
```

A `pkg` folder will be created in the root of the directory, this will contain a NPM package that can be used in Javascript.

To make it to be used for cbdi-edit, go to `pkg` folder, install `yalc` globally in your pc and publish to local yalc store

```bash
npm i yalc -g
cd pkg
yalc publish
```

For more information, please look at this tutorial : https://developer.mozilla.org/en-US/docs/WebAssembly/Rust_to_wasm
