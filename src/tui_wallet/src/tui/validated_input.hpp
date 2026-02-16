#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "general/result.hpp"
namespace ui {
using namespace ftxui;
using StringValidator = std::function<bool(std::string&)>;

struct ParsedInputBase : public ComponentBase {
public:
    std::string content;
    [[nodiscard]] virtual bool valid() const = 0;
    virtual void parse() const = 0;
    auto colored() { return color(valid() ? Color::Green : Color::Red); };
    ParsedInputBase()
    {
        InputOption option;
        option.multiline = false;
        option.content = &content;
        option.on_change = [this]() {
            parse();
        };
        option.transform = [&](InputState state) {
            auto element { state.element };
            element |= colored();
            if (state.focused) {
                return element | inverted;
            }
            return element;
        };
        Add(Input(option));
    }
};

struct LabeledParsedInputBase : public ParsedInputBase {
    std::string label;
    LabeledParsedInputBase(std::string label)
        : label(std::move(label))
    {
    }
    Element OnRender() override
    {
        return hbox({
            text(label) | colored(),
            ParsedInputBase::OnRender(),
        });
    };
};

template <typename T>
struct LabeledParsedInputBaseFor : public LabeledParsedInputBase {
private:
    Result<T>& ref;
    LabeledParsedInputBaseFor(std::string label, Result<T>& ref)
        : LabeledParsedInputBase(std::move(label))
        , ref(ref)
    {
    }
    bool valid() const override
    {
        return ref.has_value();
    }
    void parse() const override
    {
        ref = T::try_parse(content);
    }
};

template <typename T>
inline auto LabeledParsedInputFor(std::string label, Result<T>& r)
{
    return Make<LabeledParsedInputBaseFor<T>>(std::move(label), r);
}

struct ValidatedBase : public ComponentBase {
    StringValidator _validator;
    bool valid { false };
    std::string content;
    ValidatedBase(const ValidatedBase&) = delete;
    bool OnEvent(Event event) override
    {
        if (event == Event::ArrowUp || event == Event::ArrowDown)
            return false;
        return ComponentBase::OnEvent(std::move(event));
    }
    void validate()
    {
        valid = _validator(content);
    }
    void set_validator(StringValidator validator)
    {
        _validator = std::move(validator);
        validate();
    }
    ValidatedBase(StringValidator validator)
        : _validator(std::move(validator))
    {
        InputOption option;
        option.multiline = false;
        option.content = &content;
        option.on_change = [this]() {
            validate();
        };
        option.transform = [&](InputState state) {
            auto element { state.element };
            element |= color(valid ? Color::Green : Color::Red);
            if (state.focused) {
                return element | inverted;
            }
            return element;
        };
        Add(Input(option));
    }
};
inline Component Validated(StringValidator validator)
{
    return Make<ValidatedBase>(std::move(validator));
}

struct LabeledValidatedBase : public ValidatedBase {
    using ValidatedBase::ValidatedBase;
    std::string label;
    LabeledValidatedBase(std::string label, StringValidator validator)
        : ValidatedBase(std::move(validator))
        , label(std::move(label))
    {
    }
    Element OnRender() override
    {

        return hbox({
            text(label) | color(valid ? Color::Green : Color::Red),
            ValidatedBase::OnRender(),
        });
    };
};
inline std::shared_ptr<LabeledValidatedBase>
LabeledValidated(std::string label, StringValidator validator = [](std::string) { return false; })
{
    return Make<LabeledValidatedBase>(std::move(label), std::move(validator));
}
} // namespace ui
