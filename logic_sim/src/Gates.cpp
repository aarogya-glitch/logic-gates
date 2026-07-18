#include "Gates.h"

void ANDGate::evaluate()   { setOutput(getInput(0) && getInput(1)); }
void ORGate::evaluate()    { setOutput(getInput(0) || getInput(1)); }
void NOTGate::evaluate()   { setOutput(!getInput(0)); }
void NANDGate::evaluate()  { setOutput(!(getInput(0) && getInput(1))); }
void NORGate::evaluate()   { setOutput(!(getInput(0) || getInput(1))); }
void XORGate::evaluate()   { setOutput(getInput(0) != getInput(1)); }
void XNORGate::evaluate()  { setOutput(getInput(0) == getInput(1)); }
void BufferGate::evaluate(){ setOutput(getInput(0)); }
void LEDGate::evaluate()   { setOutput(getInput(0)); }
