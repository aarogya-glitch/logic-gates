#include "FileManager.h"
#include <fstream>
#include <sstream>

bool FileManager::save(const Circuit& circuit, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    const auto& gates = circuit.getGates();
    out << "GATES " << gates.size() << "\n";
    for (const auto& g : gates) {
        out << g->getId() << " " << gateTypeToString(g->getType()) << " "
            << g->position.x << " " << g->position.y << " "
            << (g->getOutput() ? 1 : 0) << "\n";
    }

    const auto& wires = circuit.getWires();
    out << "WIRES " << wires.size() << "\n";
    for (const auto& w : wires) {
        out << w.getId() << " " << w.getSourceGateId() << " "
            << w.getDestGateId() << " " << w.getDestInputIndex() << "\n";
    }
    return true;
}

bool FileManager::load(Circuit& circuit, const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    circuit.clear();

    std::string tag;
    int count;

    in >> tag >> count; // "GATES <count>"
    if (tag != "GATES") return false;
    for (int i = 0; i < count; ++i) {
        int id; std::string typeStr; float x, y; int outVal;
        in >> id >> typeStr >> x >> y >> outVal;
        GateType type = gateTypeFromString(typeStr);
        circuit.addGateWithId(id, type, Position{x, y});
        if (type == GateType::SWITCH) {
            Gate* g = circuit.findGate(id);
            if (g) g->setOutput(outVal != 0);
        }
    }

    in >> tag >> count; // "WIRES <count>"
    if (tag != "WIRES") return false;
    for (int i = 0; i < count; ++i) {
        int id, srcId, dstId, dstIdx;
        in >> id >> srcId >> dstId >> dstIdx;
        circuit.addWireWithId(id, srcId, dstId, dstIdx);
    }

    return true;
}
