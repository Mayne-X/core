#pragma once
#include "gui.hpp"

namespace ui {
struct Popup : public ftxui::ComponentBase {
protected:
    bool closed { false };

public:
    bool is_closed() const { return closed; }
};
}
