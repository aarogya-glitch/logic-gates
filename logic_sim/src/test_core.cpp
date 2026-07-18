// Quick sanity check for the core logic (no SFML needed to build/run this).
// Builds a half-adder: SW_A, SW_B -> XOR -> LED_SUM
//                                  -> AND -> LED_CARRY
// then prints its truth table and saves/reloads it from disk.
#include "Circuit.h"
#include "Simulator.h"
#include "TruthTable.h"
#include "FileManager.h"
#include <iostream>
#include <cassert>

int main() {
    Circuit circuit;

    int swA = circuit.addGate(GateType::SWITCH, {0, 0});
    int swB = circuit.addGate(GateType::SWITCH, {0, 100});
    int xorGate = circuit.addGate(GateType::XOR, {200, 0});
    int andGate = circuit.addGate(GateType::AND, {200, 100});
    int ledSum = circuit.addGate(GateType::LED, {400, 0});
    int ledCarry = circuit.addGate(GateType::LED, {400, 100});

    bool ok = true;
    ok &= circuit.addWire(swA, xorGate, 0);
    ok &= circuit.addWire(swB, xorGate, 1);
    ok &= circuit.addWire(swA, andGate, 0);
    ok &= circuit.addWire(swB, andGate, 1);
    ok &= circuit.addWire(xorGate, ledSum, 0);
    ok &= circuit.addWire(andGate, ledCarry, 0);
    assert(ok && "wiring should have succeeded");

    // Reject an invalid connection: destination pin already driven.
    bool shouldFail = circuit.addWire(swA, xorGate, 0);
    assert(!shouldFail && "duplicate wire to an already-driven pin must be rejected");

    Simulator sim(circuit);
    sim.setSwitch(swA, true);
    sim.setSwitch(swB, false);
    sim.run();
    std::cout << "A=1 B=0 -> SUM=" << circuit.findGate(ledSum)->getOutput()
              << " CARRY=" << circuit.findGate(ledCarry)->getOutput() << "\n";
    assert(circuit.findGate(ledSum)->getOutput() == true);
    assert(circuit.findGate(ledCarry)->getOutput() == false);

    sim.setSwitch(swA, true);
    sim.setSwitch(swB, true);
    sim.run();
    std::cout << "A=1 B=1 -> SUM=" << circuit.findGate(ledSum)->getOutput()
              << " CARRY=" << circuit.findGate(ledCarry)->getOutput() << "\n";
    assert(circuit.findGate(ledSum)->getOutput() == false);
    assert(circuit.findGate(ledCarry)->getOutput() == true);

    std::cout << "\nFull truth table:\n";
    TruthTable table(circuit);
    bool generated = table.generate();
    assert(generated);
    table.printToConsole();
    table.exportToCSV("half_adder.csv");
    std::cout << "\nExported half_adder.csv\n";

    // Save + reload, then confirm the reloaded circuit behaves the same.
    assert(FileManager::save(circuit, "half_adder.circuit"));
    Circuit reloaded;
    assert(FileManager::load(reloaded, "half_adder.circuit"));
    Simulator sim2(reloaded);
    sim2.setSwitch(swA, true);
    sim2.setSwitch(swB, false);
    sim2.run();
    assert(reloaded.findGate(ledSum)->getOutput() == true);
    assert(reloaded.findGate(ledCarry)->getOutput() == false);
    std::cout << "Save/load round-trip OK\n";

    std::cout << "\nAll core logic tests passed.\n";
    return 0;
}
