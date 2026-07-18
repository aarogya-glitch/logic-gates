# Digital Logic Simulator (C++ / SFML)

A GUI digital-logic simulator: drag gates onto a canvas, wire them together,
flip switches, watch LEDs light up in real time, and auto-generate a truth
table. Built to match the class/module spec you provided.

## What's implemented

| Module (from spec)          | Where it lives                              |
|------------------------------|----------------------------------------------|
| Logic Gate Library (8 gates) | `Gate.h/.cpp`, `Gates.h/.cpp`                 |
| Wire Connection Module       | `Wire.h`, `Circuit::addWire/removeWire`       |
| Input Module (switches)      | `SwitchGate` in `Gates.h`                     |
| Output Module (LEDs)         | `LEDGate` in `Gates.h`                        |
| Simulation Engine            | `Circuit::propagate()`, `Simulator.h`         |
| Truth Table Generator        | `TruthTable.h/.cpp` (+ CSV export)            |
| Save & Load Circuits         | `FileManager.h/.cpp`                          |
| Circuit Design Workspace, GUI | `src/main.cpp` (SFML)                        |

Gates implemented: AND, OR, NOT, NAND, NOR, XOR, XNOR, Buffer, plus
Switch (input) and LED (output), which are modeled as 0-input / 1-input
"gates" so they share the same base class and can live in the same
graph as the logic gates.

**The core logic has zero GUI dependency.** `Circuit`, `Gate`, `Wire`,
`Simulator`, `TruthTable`, and `FileManager` are plain C++ with no SFML
includes anywhere, and are exercised directly by `test_core.cpp` (a
console program with no window). `main.cpp` is the only file that
touches SFML, and it only draws things and translates mouse/keyboard
events into calls on `Circuit`/`Simulator` — exactly the split your
"can we use a logic gate library" answer describes: the library may
draw and handle input, but your own code must evaluate the logic.

## Building

Requires a C++17 compiler, CMake, and SFML 2.5+.

```bash
# Ubuntu/Debian
sudo apt install build-essential cmake libsfml-dev

mkdir build && cd build
cmake ..
make -j4
```

This produces two executables:

- `test_core` — console program, no GUI/SFML needed. Builds a
  half-adder circuit in code, runs it, prints its truth table, exports
  it to CSV, and round-trips a save/load. Good for grading the logic
  itself, or on a machine without SFML installed.
- `logic_sim_gui` — the interactive GUI (only built if SFML is found).

Run either from the `build/` directory:

```bash
./test_core
./logic_sim_gui
```

## Using the GUI

- **Toolbox (left):** click a gate type, then click anywhere on the
  canvas to place one.
- **Wiring:** click-drag from a gate's **output pin** (blue circle, on
  the right edge) to another gate's **input pin** (left edge) to
  connect them. An input pin can only take one wire; a pin that's
  already driven, or dropping on empty space, is rejected.
- **Moving a gate:** click-drag its body (not a pin).
- **Switches:** click a switch's body to toggle it between 0/1
  (green = 1).
- **LEDs:** shown as circles, green when lit, red when off — updated
  live every frame after propagation runs.
- **Delete:** click "Delete" in the top bar to arm delete mode, then
  click a gate to remove it (and any wires touching it). Click
  "Delete" again to turn it off.
- **New / Save / Load:** top bar. Save/Load use a fixed file,
  `circuit.save`, in the working directory.
- **Truth Table:** top bar button. Requires at least one Switch and
  one LED in the circuit; writes `truth_table.csv` and reports how
  many rows were generated in the status bar. (The GUI intentionally
  doesn't pop up a separate table window — open the CSV, or extend
  `TruthTable`/main.cpp to render one, as a next step.)
- **Esc** cancels whatever you're doing (placing a gate, dragging a
  wire).

## Simulation algorithm

`Circuit::propagate()`:
1. Push every wire's source gate output into its destination's input pin.
2. Re-evaluate every non-Switch gate from its (now updated) inputs.
3. Repeat until a full pass produces no change (stable), or a
   iteration cap is hit.

This is a simple fixed-point iteration rather than a topological sort,
which means it also handles feedback loops (e.g. a latch built from
cross-coupled NOR/NAND gates) — the iteration cap is what protects
against a genuinely unstable/invalid circuit hanging the simulator.

## Truth table generation algorithm

`TruthTable::generate()`:
1. Find every Switch (n of them) and every LED in the circuit.
2. For each of the 2^n input combinations: drive the switches to that
   combination's bits, run the simulator, record the LED outputs.
3. Rows can be printed to console or exported as CSV.

## Extending

The spec's "Future Enhancements" list (flip-flops, clock signal, MUX/DEMUX,
encoders/decoders, counters/registers, K-map simplification, PCB-style
routing, image/PDF export) all slot in the same way: add a new `GateType`
+ subclass in `Gates.h/.cpp` (or a new component class alongside `Gate`
for things like a clock), and the rest of the engine (wiring, propagation,
truth tables, save/load) doesn't need to change. A clock, for example,
could be a `Gate` subclass with 0 inputs whose `evaluate()` flips its own
output based on elapsed time, similar to `SwitchGate` but driven by a timer
in the main loop instead of a mouse click.

## File layout

```
include/        headers for the core logic (no SFML)
src/Gate.cpp, Gates.cpp, Circuit.cpp, TruthTable.cpp, FileManager.cpp
src/test_core.cpp   console test/demo, no SFML needed
src/main.cpp        SFML GUI (only file that includes SFML)
CMakeLists.txt
```
