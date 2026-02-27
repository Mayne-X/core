#pragma once

#include "ftxui/component/component_options.hpp"
#include <string>
auto PlainButton(ftxui::ConstStringRef label, auto&& on_click)
{
    ftxui::ButtonOption option;
    option.transform = [](const ftxui::EntryState& s) {
        auto element = ftxui::text(s.label);
        if (s.focused) {
            element = element | ftxui::bold | ftxui::inverted;
        }
        return element;
    };
    return Button(label, std::forward<decltype(on_click)>(on_click), option);
}
