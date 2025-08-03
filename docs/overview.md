# Hydra+ Language Overview

Hydra+ is a minimalist scripting language for Windows and Linux. It ships as a small executable containing its own loader and interpreter. Scripts are typically used for backend services, API endpoints and data processing.

## Basic Syntax
- Variables start with `$` and must be assigned on declaration, e.g. `$name = "";`.
- Comments use `#` for single line or `[ * ... * ]` for multiline. The `?` can be used as special comment for annotations but is not different that the #.
- Core control structures include `loop`, `if/else` and `switch`. Loops support `break` and `continue` statements.
- Functions are declared with `func name($params){...}`. Use `return` to provide a result.
- Objects are defined with `obj ClassName { ... }` and instantiated via `Class.ClassName`.
- Asynchronous execution in true OS threads is available via `async $id @function()` and thread management commands such as `terminate`.

## Data Types
Hydra+ provides several built-in types:
- `String` – simple UTF‑8 string managed automatically by the interpreter.
- `Integer`, `Real`, `Boolean` and `CodePoint` – numeric primitives with automatic storage.
- `List` family – dynamic collections including simple lists, string lists and fast lists.
- `Bytes` – mutable byte arrays.
- Database connections, datasets, sockets and files for system integration.

Complex strings offer efficient concatenation (`concat`) or variable expansion (`expand`). The documentation cautions that simple strings are managed automatically but can be memory hungry for large data【F:hydra+ documentation.txt†L61-L66】.

## Memory Considerations
Hydra+ does not implement a full garbage collector. Function parameters are passed by reference and objects must be freed when no longer needed. The documentation notes that improper cleanup may lead to memory spikes, particularly with large strings or datasets【F:hydra+ documentation.txt†L45-L82】.

## Control Flow Example
```hydra
$idx = 0;
loop {
  if ($idx == 100) break;
  if ($idx == 50) continue;
  echo("Index: "+$idx.ToString(0));
  $idx = $idx + 1;
}
```
This snippet illustrates the loop structure with conditional breaks and continues【F:hydra+ documentation.txt†L520-L550】.

## Objects
Objects group variables and functions for organization. They are defined in the script and instantiated as needed. Instance members are accessed with `.` just like list elements. Object methods cannot run asynchronously【F:hydra+ documentation.txt†L720-L752】.

## Threads and Async Functions
Top level functions may run in separate threads using the `async` keyword. Each thread has an identifier and can be monitored or terminated via commands like `terminate "id"` or `threadInfo("id")`. Parameters to async functions pass by a weak reference, so altering numeric types does not affect the caller directly【F:hydra+ documentation.txt†L660-L712】.

## Domain Functions
Domains provide grouped functionality similar to modules. For example, the `String` domain exposes methods such as `.Concat()` and `.Expand()` for complex strings【F:hydra+ documentation.txt†L840-L928】.

## Sample Usage
The repository includes sample scripts under `samples/` demonstrating HTTP requests and database access. The `sample_http.hydra` file shows how to POST JSON and save a response using the user defined `HTTP` class【F:samples/sample_http.hydra†L1-L33】.

For more detailed descriptions of every domain and function, see `hydra+ documentation.txt` in the repository root.
