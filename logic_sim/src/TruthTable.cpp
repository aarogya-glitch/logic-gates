#include "TruthTable.h"
#include "Simulator.h"
#include <fstream>
#include <iostream>
#include <iomanip>

bool TruthTable::generate() {
    switchIds.clear();
    ledIds.clear();
    rows.clear();

    for (Gate* g : circuit.getSwitches()) switchIds.push_back(g->getId());
    for (Gate* g : circuit.getLEDs())     ledIds.push_back(g->getId());

    if (switchIds.empty() || ledIds.empty()) return false;

    Simulator sim(circuit);
    int n = (int)switchIds.size();
    long long combos = 1LL << n; // 2^n

    for (long long mask = 0; mask < combos; ++mask) {
        std::vector<bool> row;
        row.reserve(n + ledIds.size());

        // Drive each switch to the bit of `mask` corresponding to it.
        for (int i = 0; i < n; ++i) {
            bool bit = (mask >> (n - 1 - i)) & 1; // MSB-first, matches the spec's example table
            sim.setSwitch(switchIds[i], bit);
            row.push_back(bit);
        }

        if (!sim.run()) return false; // circuit didn't stabilize - invalid circuit

        for (int ledId : ledIds) {
            Gate* led = circuit.findGate(ledId);
            row.push_back(led ? led->getOutput() : false);
        }

        rows.push_back(row);
    }
    return true;
}

void TruthTable::printToConsole() const {
    int n = (int)switchIds.size();
    for (int i = 0; i < n; ++i) std::cout << "IN" << switchIds[i] << "\t";
    for (size_t i = 0; i < ledIds.size(); ++i) std::cout << "OUT" << ledIds[i] << "\t";
    std::cout << "\n";

    for (const auto& row : rows) {
        for (bool v : row) std::cout << (v ? 1 : 0) << "\t";
        std::cout << "\n";
    }
}

bool TruthTable::exportToCSV(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    int n = (int)switchIds.size();
    for (int i = 0; i < n; ++i) out << "IN" << switchIds[i] << (i + 1 < n || !ledIds.empty() ? "," : "");
    for (size_t i = 0; i < ledIds.size(); ++i) out << "OUT" << ledIds[i] << (i + 1 < ledIds.size() ? "," : "");
    out << "\n";

    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i)
            out << (row[i] ? 1 : 0) << (i + 1 < row.size() ? "," : "");
        out << "\n";
    }
    return true;
}
