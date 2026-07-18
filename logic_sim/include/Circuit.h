#pragma once
#include "Gate.h"
#include "Wire.h"
#include <memory>
#include <vector>

// Circuit owns every Gate (via unique_ptr, so no leaks and no manual
// delete anywhere) plus the list of Wires connecting them. It is the
// "Composition" relationship called out in the assignment: a Circuit
// *has-a* collection of gates and wires.
class Circuit {
public:
    // Creates a gate of the given type at the given canvas position and
    // adds it to the circuit. Returns the new gate's id.
    int addGate(GateType type, Position pos);

    // Removes a gate and any wire touching it. Returns false if not found.
    bool removeGate(int gateId);

    // Connects sourceGateId's output to destGateId's input pin
    // destInputIndex. Returns false (and adds nothing) if either gate
    // doesn't exist, the input index is invalid, or that input pin is
    // already driven by another wire (prevents invalid connections).
    bool addWire(int sourceGateId, int destGateId, int destInputIndex);

    bool removeWire(int wireId);

    // Runs the wire -> input propagation + gate evaluation loop until the
    // circuit stabilizes or maxIterations is reached (the iteration cap
    // is what keeps a feedback loop / invalid circuit from hanging).
    // Returns true if the circuit stabilized before the cap was hit.
    bool propagate(int maxIterations = 200);

    Gate* findGate(int id) const;

    std::vector<Gate*> getSwitches() const;
    std::vector<Gate*> getLEDs() const;

    // Any (gateId, inputIndex) pair not currently driven by a wire.
    // Handy for the "detect invalid circuits" requirement.
    std::vector<std::pair<int,int>> getUnconnectedInputs() const;

    const std::vector<std::unique_ptr<Gate>>& getGates() const { return gates; }
    const std::vector<Wire>& getWires() const { return wires; }

    void clear();

    // Used by FileManager when reloading a saved circuit, so ids match
    // what was saved instead of being renumbered.
    int addGateWithId(int id, GateType type, Position pos);
    bool addWireWithId(int id, int sourceGateId, int destGateId, int destInputIndex);

private:
    std::vector<std::unique_ptr<Gate>> gates;
    std::vector<Wire> wires;
    int nextGateId = 1;
    int nextWireId = 1;

    bool isInputDriven(int gateId, int inputIndex) const;
};
