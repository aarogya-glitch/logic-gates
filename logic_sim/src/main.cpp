// Digital Logic Simulator - SFML GUI
//
// This file only handles drawing and mouse/keyboard interaction. All
// logic (gate evaluation, wiring rules, truth table generation, save/
// load) lives in Circuit / Simulator / TruthTable / FileManager, which
// have zero SFML dependency and are unit-tested separately in
// test_core.cpp. That split is what keeps "the GUI only displays
// things" true, per the project's own guidelines.
#include <SFML/Graphics.hpp>
#include "Circuit.h"
#include "Simulator.h"
#include "TruthTable.h"
#include "FileManager.h"
#include <vector>
#include <sstream>
#include <optional>

// ---- layout constants ----------------------------------------------------
static const float TOOLBOX_WIDTH = 150.f;
static const float TOPBAR_HEIGHT = 36.f;
static const float STATUSBAR_HEIGHT = 26.f;
static const float GATE_W = 70.f;
static const float GATE_H = 44.f;
static const float PIN_R = 6.f;
static const sf::Vector2u WINDOW_SIZE = {1100, 720};

struct ToolboxItem {
    GateType type;
    std::string label;
    sf::FloatRect rect;
};

// Screen-space position of a gate's input pin (index) and output pin.
static sf::Vector2f inputPinPos(const Gate* g, int index) {
    float spacing = GATE_H / (g->getNumInputs() + 1);
    return { g->position.x, g->position.y + spacing * (index + 1) };
}
static sf::Vector2f outputPinPos(const Gate* g) {
    return { g->position.x + GATE_W, g->position.y + GATE_H / 2.f };
}

static bool pointNear(sf::Vector2f p, sf::Vector2f target, float radius) {
    float dx = p.x - target.x, dy = p.y - target.y;
    return (dx * dx + dy * dy) <= radius * radius;
}

// Finds the gate whose body rectangle contains `p`, or nullptr.
static Gate* gateAt(Circuit& circuit, sf::Vector2f p) {
    for (const auto& g : circuit.getGates()) {
        sf::FloatRect body(g->position.x, g->position.y, GATE_W, GATE_H);
        if (body.contains(p)) return g.get();
    }
    return nullptr;
}

// Finds (gate, inputIndex) whose input pin is near `p`, or (nullptr,-1).
static std::pair<Gate*, int> inputPinAt(Circuit& circuit, sf::Vector2f p) {
    for (const auto& g : circuit.getGates()) {
        for (int i = 0; i < g->getNumInputs(); ++i)
            if (pointNear(p, inputPinPos(g.get(), i), PIN_R + 3.f))
                return { g.get(), i };
    }
    return { nullptr, -1 };
}

// Finds the gate whose *output* pin is near `p`, or nullptr. SWITCH/LED
// still expose exactly one output pin each (LED's is unused for wiring
// out, but we don't need to special-case that - nothing will normally
// connect from it).
static Gate* outputPinAt(Circuit& circuit, sf::Vector2f p) {
    for (const auto& g : circuit.getGates())
        if (pointNear(p, outputPinPos(g.get()), PIN_R + 3.f))
            return g.get();
    return nullptr;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE.x, WINDOW_SIZE.y),
                             "Digital Logic Simulator");
    window.setFramerateLimit(60);

    sf::Font font;
    bool haveFont = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    Circuit circuit;
    Simulator sim(circuit);

    // ---- toolbox ----------------------------------------------------------
    std::vector<ToolboxItem> toolbox;
    {
        std::vector<std::pair<GateType, std::string>> defs = {
            {GateType::AND, "AND"}, {GateType::OR, "OR"}, {GateType::NOT, "NOT"},
            {GateType::NAND, "NAND"}, {GateType::NOR, "NOR"}, {GateType::XOR, "XOR"},
            {GateType::XNOR, "XNOR"}, {GateType::BUFFER, "BUFFER"},
            {GateType::SWITCH, "SWITCH"}, {GateType::LED, "LED"},
        };
        float y = TOPBAR_HEIGHT + 10.f;
        for (auto& d : defs) {
            ToolboxItem item{ d.first, d.second, sf::FloatRect(10.f, y, TOOLBOX_WIDTH - 20.f, 34.f) };
            toolbox.push_back(item);
            y += 42.f;
        }
    }

    // ---- top bar buttons ----------------------------------------------------
    sf::FloatRect btnNew(10, 4, 60, 28);
    sf::FloatRect btnSave(80, 4, 60, 28);
    sf::FloatRect btnLoad(150, 4, 60, 28);
    sf::FloatRect btnTruth(220, 4, 110, 28);
    sf::FloatRect btnDelete(340, 4, 90, 28);

    GateType pendingPlaceType = GateType::AND;
    bool placing = false;
    bool deleteMode = false;

    Gate* wireDragSource = nullptr; // dragging a new wire from this gate's output
    sf::Vector2f wireDragCurrent;

    Gate* dragGate = nullptr;       // moving an existing gate
    sf::Vector2f dragOffset;

    std::string statusText = "Ready. Click a toolbox item, then click the canvas to place it.";

    auto setStatus = [&](const std::string& s) { statusText = s; };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mp(event.mouseButton.x, event.mouseButton.y);

                // Top bar buttons
                if (btnNew.contains(mp)) {
                    circuit.clear();
                    setStatus("New circuit.");
                } else if (btnSave.contains(mp)) {
                    FileManager::save(circuit, "circuit.save");
                    setStatus("Saved to circuit.save");
                } else if (btnLoad.contains(mp)) {
                    if (FileManager::load(circuit, "circuit.save"))
                        setStatus("Loaded circuit.save");
                    else
                        setStatus("Could not load circuit.save (not found?)");
                } else if (btnTruth.contains(mp)) {
                    TruthTable table(circuit);
                    if (table.generate()) {
                        table.exportToCSV("truth_table.csv");
                        setStatus("Truth table generated -> truth_table.csv (" +
                                  std::to_string(table.getRows().size()) + " rows)");
                    } else {
                        setStatus("Need >=1 SWITCH and >=1 LED to build a truth table.");
                    }
                } else if (btnDelete.contains(mp)) {
                    // Toggle delete-mode: next click on a gate removes it.
                    deleteMode = !deleteMode;
                    placing = false;
                    setStatus(deleteMode ? "Delete mode: click a gate to remove it (and its wires)."
                                          : "Delete mode off.");
                }
                // Toolbox
                else if (mp.x < TOOLBOX_WIDTH) {
                    for (auto& item : toolbox) {
                        if (item.rect.contains(mp)) {
                            placing = true;
                            deleteMode = false;
                            pendingPlaceType = item.type;
                            setStatus("Placing " + item.label + " - click the canvas.");
                        }
                    }
                }
                // Canvas
                else if (mp.y > TOPBAR_HEIGHT) {
                    if (deleteMode) {
                        if (Gate* g = gateAt(circuit, mp)) {
                            circuit.removeGate(g->getId());
                            setStatus("Deleted gate.");
                        }
                    } else if (placing) {
                        circuit.addGate(pendingPlaceType, Position{ mp.x, mp.y });
                        placing = false;
                        setStatus("Placed. Drag from an output pin (right, blue) to an "
                                  "input pin (left) to wire them.");
                    } else {
                        // Try output pin first (start a wire), then gate body (move),
                        // then toggle if it's a switch.
                        if (Gate* src = outputPinAt(circuit, mp)) {
                            wireDragSource = src;
                            wireDragCurrent = mp;
                        } else if (Gate* g = gateAt(circuit, mp)) {
                            if (g->getType() == GateType::SWITCH) {
                                g->setOutput(!g->getOutput());
                                setStatus("Toggled switch to " + std::string(g->getOutput() ? "1" : "0"));
                            } else {
                                dragGate = g;
                                dragOffset = { mp.x - g->position.x, mp.y - g->position.y };
                            }
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mp((float)event.mouseMove.x, (float)event.mouseMove.y);
                if (wireDragSource) wireDragCurrent = mp;
                if (dragGate) {
                    dragGate->position.x = mp.x - dragOffset.x;
                    dragGate->position.y = mp.y - dragOffset.y;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mp((float)event.mouseButton.x, (float)event.mouseButton.y);
                if (wireDragSource) {
                    auto [destGate, destIdx] = inputPinAt(circuit, mp);
                    if (destGate) {
                        bool ok = circuit.addWire(wireDragSource->getId(), destGate->getId(), destIdx);
                        setStatus(ok ? "Wire connected." : "Invalid connection (pin busy, or bad target).");
                    }
                    wireDragSource = nullptr;
                }
                dragGate = nullptr;
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                placing = false;
                wireDragSource = nullptr;
                dragGate = nullptr;
                setStatus("Cancelled.");
            }
        }

        // Continuously keep outputs in sync with inputs/switches.
        sim.run();

        // ---- drawing ----------------------------------------------------------
        window.clear(sf::Color(30, 32, 38));

        // grid
        for (int x = (int)TOOLBOX_WIDTH; x < (int)WINDOW_SIZE.x; x += 25) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((float)x, TOPBAR_HEIGHT), sf::Color(45, 47, 54)),
                sf::Vertex(sf::Vector2f((float)x, WINDOW_SIZE.y - STATUSBAR_HEIGHT), sf::Color(45, 47, 54))
            };
            window.draw(line, 2, sf::Lines);
        }
        for (int y = (int)TOPBAR_HEIGHT; y < (int)(WINDOW_SIZE.y - STATUSBAR_HEIGHT); y += 25) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(TOOLBOX_WIDTH, (float)y), sf::Color(45, 47, 54)),
                sf::Vertex(sf::Vector2f((float)WINDOW_SIZE.x, (float)y), sf::Color(45, 47, 54))
            };
            window.draw(line, 2, sf::Lines);
        }

        // top bar
        sf::RectangleShape topBar({ (float)WINDOW_SIZE.x, TOPBAR_HEIGHT });
        topBar.setFillColor(sf::Color(24, 26, 31));
        window.draw(topBar);

        auto drawButton = [&](sf::FloatRect r, const std::string& label, sf::Color color) {
            sf::RectangleShape rect({ r.width, r.height });
            rect.setPosition(r.left, r.top);
            rect.setFillColor(color);
            rect.setOutlineColor(sf::Color(90, 90, 100));
            rect.setOutlineThickness(1.f);
            window.draw(rect);
            if (haveFont) {
                sf::Text t(label, font, 13);
                t.setPosition(r.left + 6, r.top + 6);
                t.setFillColor(sf::Color::White);
                window.draw(t);
            }
        };
        drawButton(btnNew, "New", sf::Color(60, 62, 70));
        drawButton(btnSave, "Save", sf::Color(60, 62, 70));
        drawButton(btnLoad, "Load", sf::Color(60, 62, 70));
        drawButton(btnTruth, "Truth Table", sf::Color(60, 90, 70));
        drawButton(btnDelete, "Delete", deleteMode ? sf::Color(150, 60, 60) : sf::Color(90, 60, 60));

        // toolbox panel
        sf::RectangleShape toolboxPanel({ TOOLBOX_WIDTH, (float)WINDOW_SIZE.y - TOPBAR_HEIGHT });
        toolboxPanel.setPosition(0, TOPBAR_HEIGHT);
        toolboxPanel.setFillColor(sf::Color(24, 26, 31));
        window.draw(toolboxPanel);

        for (auto& item : toolbox) {
            sf::RectangleShape rect({ item.rect.width, item.rect.height });
            rect.setPosition(item.rect.left, item.rect.top);
            bool selected = placing && item.type == pendingPlaceType;
            rect.setFillColor(selected ? sf::Color(70, 100, 140) : sf::Color(50, 52, 60));
            rect.setOutlineColor(sf::Color(90, 90, 100));
            rect.setOutlineThickness(1.f);
            window.draw(rect);
            if (haveFont) {
                sf::Text t(item.label, font, 14);
                t.setPosition(item.rect.left + 8, item.rect.top + 8);
                t.setFillColor(sf::Color::White);
                window.draw(t);
            }
        }

        // wires
        for (const auto& w : circuit.getWires()) {
            Gate* src = circuit.findGate(w.getSourceGateId());
            Gate* dst = circuit.findGate(w.getDestGateId());
            if (!src || !dst) continue;
            sf::Vector2f a = outputPinPos(src);
            sf::Vector2f b = inputPinPos(dst, w.getDestInputIndex());
            sf::Color wireColor = src->getOutput() ? sf::Color(120, 220, 120) : sf::Color(140, 140, 150);
            sf::Vertex line[] = { sf::Vertex(a, wireColor), sf::Vertex(b, wireColor) };
            window.draw(line, 2, sf::Lines);
        }

        // in-progress wire being dragged
        if (wireDragSource) {
            sf::Vertex line[] = {
                sf::Vertex(outputPinPos(wireDragSource), sf::Color(200, 200, 100)),
                sf::Vertex(wireDragCurrent, sf::Color(200, 200, 100))
            };
            window.draw(line, 2, sf::Lines);
        }

        // gates
        for (const auto& gPtr : circuit.getGates()) {
            Gate* g = gPtr.get();
            sf::Vector2f pos(g->position.x, g->position.y);

            if (g->getType() == GateType::LED) {
                sf::CircleShape circle(GATE_H / 2.f);
                circle.setPosition(pos.x, pos.y);
                circle.setFillColor(g->getOutput() ? sf::Color(80, 230, 80) : sf::Color(70, 40, 40));
                circle.setOutlineColor(sf::Color(200, 200, 200));
                circle.setOutlineThickness(2.f);
                window.draw(circle);
            } else if (g->getType() == GateType::SWITCH) {
                sf::RectangleShape rect({ GATE_W * 0.6f, GATE_H });
                rect.setPosition(pos);
                rect.setFillColor(g->getOutput() ? sf::Color(80, 160, 90) : sf::Color(90, 60, 60));
                rect.setOutlineColor(sf::Color(200, 200, 200));
                rect.setOutlineThickness(2.f);
                window.draw(rect);
            } else {
                sf::RectangleShape rect({ GATE_W, GATE_H });
                rect.setPosition(pos);
                rect.setFillColor(sf::Color(55, 90, 130));
                rect.setOutlineColor(sf::Color(200, 200, 200));
                rect.setOutlineThickness(dragGate == g ? 3.f : 1.5f);
                window.draw(rect);
            }

            if (haveFont) {
                sf::Text t(g->getShortLabel(), font, 13);
                t.setFillColor(sf::Color::White);
                t.setPosition(pos.x + 6, pos.y + GATE_H / 2.f - 8);
                window.draw(t);
            }

            // input pins
            for (int i = 0; i < g->getNumInputs(); ++i) {
                sf::CircleShape pin(PIN_R);
                sf::Vector2f pp = inputPinPos(g, i);
                pin.setPosition(pp.x - PIN_R, pp.y - PIN_R);
                pin.setFillColor(g->getInput(i) ? sf::Color(120, 220, 120) : sf::Color(150, 150, 160));
                window.draw(pin);
            }
            // output pin (LED has none meaningful to wire *from*, but harmless to draw)
            {
                sf::CircleShape pin(PIN_R);
                sf::Vector2f pp = outputPinPos(g);
                pin.setPosition(pp.x - PIN_R, pp.y - PIN_R);
                pin.setFillColor(sf::Color(100, 160, 230));
                window.draw(pin);
            }
        }

        // status bar
        sf::RectangleShape statusBar({ (float)WINDOW_SIZE.x, STATUSBAR_HEIGHT });
        statusBar.setPosition(0, WINDOW_SIZE.y - STATUSBAR_HEIGHT);
        statusBar.setFillColor(sf::Color(24, 26, 31));
        window.draw(statusBar);
        if (haveFont) {
            sf::Text t(statusText, font, 13);
            t.setPosition(8, WINDOW_SIZE.y - STATUSBAR_HEIGHT + 5);
            t.setFillColor(sf::Color(210, 210, 210));
            window.draw(t);
        }

        window.display();
    }

    return 0;
}
