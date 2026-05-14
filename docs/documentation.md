# SwiftCode CLI

SwiftCode currently uses `--verbose` as the active generator command. Other
commands are kept in the CLI for compatibility, but they are deprecated and do
not run generation.

## Active command

```text
swiftcode --verbose <inputdir> <outputdir>
```

Scans C++ source files in `<inputdir>` and writes generated headers to
`<outputdir>` with the same relative file names.

Main rules:
- Mark enum classes on the same declaration line:
  ```cpp
  enum class Subsystem : uint8_t { // Verbose needed
  ```
- After generation, the source marker becomes:
  ```cpp
  // Verbosed version 1. Need update(yes/no): no
  ```
- Change `no` to `yes` when the generated verbose code must be regenerated.
- Generated code keeps the enum namespace and emits an enum name lookup array
  plus `std::ostream& operator<<`.
- Keep `<inputdir>` and `<outputdir>` different. The generator rejects output
  files that would overwrite input sources.

Tips:
- Include the generated header where `operator<<` support is needed.
- Generated output prints enum item names, not numeric casts, for known values.
- Unknown enum values fall back to their integer value.

## Deprecated commands

These commands are deprecated and currently print a warning without running
their old generators.

```text
swiftcode -sql <raw_sql_dir> <gen_sql_dir>
```

Old raw SQL header generation.

```text
swiftcode -enum <source_dir>
```

Old enum array generation inside source files.

```text
swiftcode -enum-domain <contract_dir> <database_dir>
```

Old enum-domain SQL generation. This path is not planned for near-term use
because ODB ORM support is handled in the main projects.
