#include "Circuit.h"
#include "Gates.h"
#include <algorithm>

int Circuit::addGate(GateType type, Position pos) {
    return addGateWithId(nextGateId, type, pos);
}

int Circuit::addGateWithId(int id, GateType type, Position pos) {
    Gate* g = createGate(id, type);
    g->position = pos;
    gates.emplace_back(g);
    if (id >= nextGateId) nextGateId = id + 1;
    return id;
}

bool Circuit::removeGate(int gateId) {
    auto it = std::find_if(gates.begin(), gates.end(),
        [&](const std::unique_ptr<Gate>& g) { return g->getId() == gateId; });
    if (it == gates.end()) return false;

    // Remove any wire touching this gate first, so we never leave a
    // dangling reference to a deleted gate.
    wires.erase(std::remove_if(wires.begin(), wires.end(), [&](const Wire& w) {
        return w.getSourceGateId() == gateId || w.getDestGateId() == gateId;
    }), wires.end());

    gates.erase(it);
    return true;
}

bool Circuit::isInputDriven(int gateId, int inputIndex) const {
    for (const auto& w : wires)
        if (w.getDestGateId() == gateId && w.getDestInputIndex() == inputIndex)
            return true;
    return false;
}

bool Circuit::addWire(int sourceGateId, int destGateId, int destInputIndex) {
    return addWireWithId(nextWireId, sourceGateId, destGateId, destInputIndex);
}

bool Circuit::addWireWithId(int id, int sourceGateId, int destGateId, int destInputIndex) {
    Gate* src = findGate(sourceGateId);
    Gate* dst = findGate(destGateId);
    if (!src || !dst) return false;                                  // gate doesn't exist
    if (destInputIndex < 0 || destInputIndex >= dst->getNumInputs())  // bad pin index
        return false;
    if (isInputDriven(destGateId, destInputIndex))                   // pin already wired
        return false;
    if (sourceGateId == destGateId)                                  // no self-loops on one pin
        return false;

    wires.emplace_back(id, sourceGateId, destGateId, destInputIndex);
    if (id >= nextWireId) nextWireId = id + 1;
    return true;
}

bool Circuit::removeWire(int wireId) {
    auto it = std::find_if(wires.begin(), wires.end(),
        [&](const Wire& w) { return w.getId() == wireId; });
    if (it == wires.end()) return false;
    wires.erase(it);
    return true;
}

Gate* Circuit::findGate(int id) const {
    for (const auto& g : gates)
        if (g->getId() == id) return g.get();
    return nullptr;
}

std::vector<Gate*> Circuit::getSwitches() const {
    std::vector<Gate*> result;
    for (const auto& g : gates)
        if (g->getType() == GateType::SWITCH) result.push_back(g.get());
    return result;
}

std::vector<Gate*> Circuit::getLEDs() const {
    std::vector<Gate*> result;
    for (const auto& g : gates)
        if (g->getType() == GateType::LED) result.push_back(g.get());
    return result;
}

std::vector<std::pair<int,int>> Circuit::getUnconnectedInputs() const {
    std::vector<std::pair<int,int>> result;
    for (const auto& g : gates)
        for (int i = 0; i < g->getNumInputs(); ++i)
            if (!isInputDriven(g->getId(), i))
                result.emplace_back(g->getId(), i);
    return result;
}

// --- The simulation algorithm --------------------------------------------
// 1. Push every wire's source output into its destination's input pin.
// 2. Re-evaluate every non-SWITCH gate from its (now updated) inputs.
// 3. Repeat until a full pass produces no change (stable), or we hit
//    maxIterations (guards against feedback loops in an invalid circuit).
// This is simple, robust to any topology (no need to topologically sort),
// and naturally supports feedback (e.g. a latch built from cross-coupled
// NOR/NAND gates), which a strict single-pass DAG evaluator could not.
bool Circuit::propagate(int maxIterations) {
    for (int iter = 0; iter < maxIterations; ++iter) {
        for (const auto& w : wires) {
            Gate* src = findGate(w.getSourceGateId());
            Gate* dst = findGate(w.getDestGateId());
            if (src && dst) dst->setInput(w.getDestInputIndex(), src->getOutput());
        }

        bool changed = false;
        for (auto& g : gates) {
            if (g->getType() == GateType::SWITCH) continue; // driven manually
            bool before = g->getOutput();
            g->evaluate();
            if (g->getOutput() != before) changed = true;
        }

        if (!changed) return true; // stabilized
    }
    return false; // did not stabilize within maxIterations
}

void Circuit::clear() {
    gates.clear();
    wires.clear();
    nextGateId = 1;
    nextWireId = 1;
}
