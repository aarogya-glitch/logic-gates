#pragma once
#include "Circuit.h"
#include <string>
#include <vector>

// Detects every SWITCH (input) and LED (output) in a Circuit, drives
// every 2^n input combination through the simulator, and records the
// resulting output row - i.e. the "Truth Table Generation" algorithm
// from the spec:
//   1. Count input switches (n)
//   2. Generate all 2^n input combinations
//   3. Simulate the circuit for each combination
//   4. Record outputs
//   5. Display the complete truth table
class TruthTable {
public:
    explicit TruthTable(Circuit& circuit) : circuit(circuit) {}

    // Populates the table. Returns false if the circuit has no switches
    // or no LEDs (nothing meaningful to tabulate), or if it never
    // stabilizes for some combination.
    bool generate();

    void printToConsole() const;
    bool exportToCSV(const std::string& filename) const;

    const std::vector<int>& getSwitchIds() const { return switchIds; }
    const std::vector<int>& getLedIds() const { return ledIds; }
    // Each row: switch values..., then LED values..., in the same order
    // as switchIds / ledIds.
    const std::vector<std::vector<bool>>& getRows() const { return rows; }

private:
    Circuit& circuit;
    std::vector<int> switchIds;
    std::vector<int> ledIds;
    std::vector<std::vector<bool>> rows;
};
