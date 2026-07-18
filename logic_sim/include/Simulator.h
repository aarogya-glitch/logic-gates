#pragma once
#include "Circuit.h"

// Thin façade over Circuit::propagate(). Kept as its own class (as the
// spec's class diagram requests) so the GUI layer talks to "the
// simulator" rather than reaching into Circuit's internals directly.
class Simulator {
public:
    explicit Simulator(Circuit& circuit) : circuit(circuit) {}

    // Re-runs propagation after an input changed (switch toggled, wire
    // added/removed, etc). Returns false if the circuit didn't stabilize
    // (likely an unintended combinational feedback loop).
    bool run() { return circuit.propagate(); }

    void setSwitch(int gateId, bool value) {
        Gate* g = circuit.findGate(gateId);
        if (g && g->getType() == GateType::SWITCH) g->setOutput(value);
    }

    void toggleSwitch(int gateId) {
        Gate* g = circuit.findGate(gateId);
        if (g && g->getType() == GateType::SWITCH) g->setOutput(!g->getOutput());
    }

private:
    Circuit& circuit;
};
