#pragma once
#include "Gate.h"

// A Wire connects one gate's single output pin to one specific input
// pin (by index) of another gate. Gate ids are stored (not raw
// pointers) so the whole circuit can be serialized/deserialized safely.
class Wire {
public:
    Wire(int id, int sourceGateId, int destGateId, int destInputIndex)
        : id(id), sourceGateId(sourceGateId), destGateId(destGateId),
          destInputIndex(destInputIndex) {}

    int getId() const { return id; }
    int getSourceGateId() const { return sourceGateId; }
    int getDestGateId() const { return destGateId; }
    int getDestInputIndex() const { return destInputIndex; }

private:
    int id;
    int sourceGateId;
    int destGateId;
    int destInputIndex;
};
