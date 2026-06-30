# Araneae

A small **compiler for a toy programming language** (`.ae` files), written in C.
It takes a Pascal/Oberon-flavored, object-oriented, **dynamically typed** language and
compiles it down to bytecode for a custom register/stack virtual machine.

This is a **purely educational project**. Its only goal was to understand, end to end and
hands-on, how a real toolchain fits together: how a language goes from source text, through
a lexer and parser, an intermediate representation, code generation, and finally down to
something a virtual machine can execute — and how the lowest layers of *system software*
(calling conventions, stack frames, interrupts, a tiny scheduler) actually work underneath.

---

## ⚠️ Important note about the design (please read)

Several of the most visible design decisions in this project were **not mine** — they were
**requirements imposed by the university course** this was written for, not choices I would
necessarily have made on my own:

- **The language is dynamically typed.** This was mandated by the course.
- **The target virtual machine** (the "ARARCH" platform the semantics were written against)
  was **provided by and required by the course** — I did not pick or design it.
- **The surface syntax** (the `begin … end;`, `method`, `:=`, trailing-semicolon style, etc.)
  was likewise **prescribed**, not something I chose.

So if some of these decisions look unusual, that is why. What *is* mine is the implementation:
the lexer, parser, AST, control-flow / DAG intermediate representation, the code generator,
and the tiny round-robin scheduler. Treat this repository as a learning exercise, not as a language I am
proposing for real use.

---

## What the language looks like

The language is small but real enough to write non-trivial programs: it has functions
(`method`), classes with fields and single inheritance, arrays, the usual control flow
(`if/then/else`, `while/do`, `repeat/until`, `break`), arithmetic/logic/bitwise operators,
and character/integer literals.

A recursive Fibonacci:

```
method fib(MaxN, CurN, A, B)
begin
  if CurN < MaxN; then
    begin
      ret fib(MaxN, CurN + 1, A + B, A);
    end;
  ret B;
end;

method main()
begin
  Res := fib(50, 0, 1, 0);
  ret Res;
end;
```

A class with methods (visibility, constructors, `this`):

```
class Calculator
  var Accumulator;
begin
  public method Calculator()
  begin
    this.Accumulator := 0;
  end;

  public method add(Term)
  begin
    this.Accumulator := this.Accumulator + Term;
    ret this.Accumulator;
  end;
end;

method main()
begin
  Calc := Calculator();
  ret Calc.add(40);
end;
```

More examples live in [test/e2e/](test/e2e/) (arithmetic, I/O, classes, arrays) and in
[kernel/lib/](kernel/lib/), which contains small "operating-system-ish" demos written in the
language plus a little inline VM assembly — including a **cooperative scheduler** and a
**round-robin (preemptive) scheduler** with context saving/restoring.

## How it works (compiler pipeline)

```
 source.ae
    │  Flex lexer            lib/lexer/lexer.l
    V
 tokens
    │  Bison parser          lib/parser/parser.y
    V
 AST                         lib/ast/        (can be dumped to Graphviz .dot)
    │  CFG / module builder  lib/cfg/
    V
 per-function control-flow graphs + expression DAGs
    │  code generation       lib/codegen/
    V
 ARARCH assembly <--- ISA semantics written by myself (ARARCH.target.pdsl.in)
```

The backend targets **ARARCH**, the custom virtual machine required by the course. Code
generation lives in [lib/codegen/Codegen.c](lib/codegen/Codegen.c) and is driven by the
`araneae` tool.

### Dynamic typing, concretely

Because the language is dynamically typed, every value at runtime is a **tagged 64-bit
reference**: the high bits carry a type tag and flags, and the low 32 bits carry the payload
(an immediate, or an offset into the stack / heap / data section). The encoding is documented
in [include/araneae/abi/Reference.h](include/araneae/abi/Reference.h) and the type tags in
[include/araneae/abi/Types.h](include/araneae/abi/Types.h). The VM checks tags at runtime and
raises errors (bad reference, type mismatch, division by zero, overflow, …) accordingly.

## Repository layout

| Path | Contents |
|------|----------|
| [lib/lexer/](lib/lexer/), [lib/parser/](lib/parser/) | Flex/Bison front end |
| [lib/ast/](lib/ast/) | Abstract syntax tree |
| [lib/cfg/](lib/cfg/) | Control-flow graphs, expression DAGs, classes, modules |
| [lib/codegen/](lib/codegen/) | ARARCH code generator |
| [lib/aux/](lib/aux/), [lib/error/](lib/error/) | Helpers, code buffers, error collection |
| [include/araneae/](include/araneae/) | Public headers, including the runtime ABI |
| [tools/araneae/](tools/araneae/) | CLI driver for the ARARCH backend |
| [tools/araneaeVM/](tools/araneaeVM/) | VM target description + helper run scripts |
| [test/e2e/](test/e2e/) | Example programs and reference output |
| [kernel/lib/](kernel/lib/) | OS-style demos (schedulers, I/O) |
| [external/](external/) | Vendored [C-Macro-Collections](https://github.com/LeoVen/C-Macro-Collections) (data structures) |

## Building

You need a C toolchain, **CMake ≥ 3.28**, **Flex**, and **Bison**.

```bash
# fetch the data-structure dependency (git submodule)
git submodule update --init --recursive

# configure & build
cmake -S . -B build
cmake --build build
```

This produces the compiler driver (under `build/`):

- `araneae` — front end + **ARARCH** code generation

## Usage

The `araneae` tool emits ARARCH assembly (and also dumps AST/CFG Graphviz files into the
output directory):

```bash
araneae <source.ae> <output-name> <output-dir>
# e.g.
araneae test/e2e/fib.ae fib build/out
# -> build/out/fib.s   (ARARCH assembly)
```

### Running on the virtual machine

The ARARCH VM itself is the **course-provided platform**, driven through an external manager
runner. The architecture is described in
[tools/araneaeVM/ARARCH.target.pdsl.in](tools/araneaeVM/ARARCH.target.pdsl.in) (instruction
semantics, error handling, the tagged-reference operand decoding) and the I/O devices in
[tools/araneaeVM/devices.xml](tools/araneaeVM/devices.xml) (a PIC, a timer, and a stdio pipe).
The helper scripts in [tools/araneaeVM/](tools/araneaeVM/) (`run.sh`, `run-io-interactive.sh`,
`run-debug.sh`, …) wrap that runner; they reference local, course-specific paths and
credentials, so you would need to adapt them to your own environment.

## Status & scope

This is exploratory, course-driven code - not production software. There are rough
edges (hardcoded paths, minimal CLI parsing, fixed-size tables, TODOs in the source). It exists
to **learn**, and it is shared in that spirit. If it helps someone else see how the pieces of a
compiler and a virtual machine fit together, all the better.

## License

The bundled `external/C-Macro-Collections` library is distributed under its own (MIT) license.
The rest of this repository is provided as-is for educational purposes.
