#pragma once
#include "Circuit.h"
#include <string>

// Saves/loads a Circuit to a small, human-readable custom text format
// (no external JSON library needed - keeping the whole project
// dependency-free besides the GUI toolkit, per the assignment's spirit
// of implementing things yourself). File layout:
//
//   GATES <count>
//   <id> <type> <x> <y> <output(0/1, only meaningful for SWITCH)>
//   ...
//   WIRES <count>
//   <id> <sourceGateId> <destGateId> <destInputIndex>
//   ...
class FileManager {
public:
    static bool save(const Circuit& circuit, const std::string& filename);
    static bool load(Circuit& circuit, const std::string& filename);
};
