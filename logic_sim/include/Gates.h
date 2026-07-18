#pragma once
#include "Gate.h"

// Each concrete gate only needs a constructor (fixes numInputs and type)
// and an evaluate() override. This is the polymorphism the assignment
// asks for: Circuit::propagate() just calls gate->evaluate() on a
// Gate*, without knowing which concrete type it is.

class ANDGate : public Gate {
public:
    explicit ANDGate(int id) : Gate(id, GateType::AND, 2) {}
    void evaluate() override;
};

class ORGate : public Gate {
public:
    explicit ORGate(int id) : Gate(id, GateType::OR, 2) {}
    void evaluate() override;
};

class NOTGate : public Gate {
public:
    explicit NOTGate(int id) : Gate(id, GateType::NOT, 1) {}
    void evaluate() override;
};

class NANDGate : public Gate {
public:
    explicit NANDGate(int id) : Gate(id, GateType::NAND, 2) {}
    void evaluate() override;
};

class NORGate : public Gate {
public:
    explicit NORGate(int id) : Gate(id, GateType::NOR, 2) {}
    void evaluate() override;
};

class XORGate : public Gate {
public:
    explicit XORGate(int id) : Gate(id, GateType::XOR, 2) {}
    void evaluate() override;
};

class XNORGate : public Gate {
public:
    explicit XNORGate(int id) : Gate(id, GateType::XNOR, 2) {}
    void evaluate() override;
};

class BufferGate : public Gate {
public:
    explicit BufferGate(int id) : Gate(id, GateType::BUFFER, 1) {}
    void evaluate() override;
};

// A SWITCH has no inputs; its output is toggled manually (by the user
// clicking it in the GUI, or by TruthTable when enumerating combinations).
class SwitchGate : public Gate {
public:
    explicit SwitchGate(int id) : Gate(id, GateType::SWITCH, 0) {}
    void evaluate() override {} // output is set externally via setOutput()
    void toggle() { setOutput(!getOutput()); }
};

// An LED has exactly one input and simply mirrors it as its output,
// which the GUI reads to decide whether to draw it lit (green) or off.
class LEDGate : public Gate {
public:
    explicit LEDGate(int id) : Gate(id, GateType::LED, 1) {}
    void evaluate() override;
};
