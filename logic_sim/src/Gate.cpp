#include "Gate.h"
#include "Gates.h"
#include <stdexcept>

Gate::Gate(int id, GateType type, int numInputs)
    : id(id), type(type), numInputs(numInputs), inputValues(numInputs, false) {}

void Gate::setInput(int index, bool value) {
    if (index < 0 || index >= (int)inputValues.size())
        throw std::out_of_range("Gate::setInput index out of range");
    inputValues[index] = value;
}

bool Gate::getInput(int index) const {
    if (index < 0 || index >= (int)inputValues.size())
        throw std::out_of_range("Gate::getInput index out of range");
    return inputValues[index];
}

std::string Gate::getLabel() const {
    return gateTypeToString(type);
}

std::string Gate::getShortLabel() const {
    switch (type) {
        case GateType::BUFFER: return "BUF";
        case GateType::SWITCH: return "SW";
        default: return gateTypeToString(type);
    }
}

std::string gateTypeToString(GateType type) {
    switch (type) {
        case GateType::AND:    return "AND";
        case GateType::OR:     return "OR";
        case GateType::NOT:    return "NOT";
        case GateType::NAND:   return "NAND";
        case GateType::NOR:    return "NOR";
        case GateType::XOR:    return "XOR";
        case GateType::XNOR:   return "XNOR";
        case GateType::BUFFER: return "BUFFER";
        case GateType::SWITCH: return "SWITCH";
        case GateType::LED:    return "LED";
    }
    return "?";
}

GateType gateTypeFromString(const std::string& s) {
    if (s == "AND")    return GateType::AND;
    if (s == "OR")     return GateType::OR;
    if (s == "NOT")    return GateType::NOT;
    if (s == "NAND")   return GateType::NAND;
    if (s == "NOR")    return GateType::NOR;
    if (s == "XOR")    return GateType::XOR;
    if (s == "XNOR")   return GateType::XNOR;
    if (s == "BUFFER") return GateType::BUFFER;
    if (s == "SWITCH") return GateType::SWITCH;
    if (s == "LED")    return GateType::LED;
    throw std::invalid_argument("Unknown gate type string: " + s);
}

Gate* createGate(int id, GateType type) {
    switch (type) {
        case GateType::AND:    return new ANDGate(id);
        case GateType::OR:     return new ORGate(id);
        case GateType::NOT:    return new NOTGate(id);
        case GateType::NAND:   return new NANDGate(id);
        case GateType::NOR:    return new NORGate(id);
        case GateType::XOR:    return new XORGate(id);
        case GateType::XNOR:   return new XNORGate(id);
        case GateType::BUFFER: return new BufferGate(id);
        case GateType::SWITCH: return new SwitchGate(id);
        case GateType::LED:    return new LEDGate(id);
    }
    return nullptr;
}
