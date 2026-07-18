#pragma once
#include <vector>
#include <string>

// Types of components supported by the simulator.
// SWITCH and LED are treated as special "gates" with 0/1 inputs
// so they can live in the same Circuit graph as the logic gates.
enum class GateType {
    AND, OR, NOT, NAND, NOR, XOR, XNOR, BUFFER, SWITCH, LED
};

// Simple 2D position, used only for GUI placement. Kept here (instead of
// pulling in SFML) so the core logic library has zero GUI dependencies.
struct Position {
    float x = 0.f;
    float y = 0.f;
};

// Abstract base class for every component placed on the workspace.
// Concrete gates only need to implement evaluate().
class Gate {
public:
    Gate(int id, GateType type, int numInputs);
    virtual ~Gate() = default;

    // Recomputes outputValue from the current inputValues.
    virtual void evaluate() = 0;

    void setInput(int index, bool value);
    bool getInput(int index) const;

    bool getOutput() const { return outputValue; }
    // Used by SWITCH gates (and by the GUI) to force an output value.
    void setOutput(bool value) { outputValue = value; }

    int getId() const { return id; }
    GateType getType() const { return type; }
    int getNumInputs() const { return numInputs; }

    // Full type name, e.g. "AND", "NAND", "SWITCH", "LED" - used for
    // save/load round-tripping via gateTypeFromString().
    std::string getLabel() const;
    // Short label for compact GUI rendering, e.g. "BUF" for BUFFER, "SW" for SWITCH.
    std::string getShortLabel() const;

    Position position;

protected:
    int id;
    GateType type;
    int numInputs;
    std::vector<bool> inputValues;
    bool outputValue = false;
};

// Helper used by Circuit (and FileManager) to construct a gate of a given
// type without the caller needing a big switch statement everywhere.
Gate* createGate(int id, GateType type);

// String <-> enum helpers (used by save/load and the GUI toolbox).
std::string gateTypeToString(GateType type);
GateType gateTypeFromString(const std::string& s);
