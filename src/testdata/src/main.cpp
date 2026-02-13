// #include
//
#include "general/funds.hpp"
#include <print>

void print_results(std::string_view s)
{
    std::println("=================");
    std::println("Test case: s = \"{}\"", s);
    if (auto pf { ParsedFunds::parse(s) }) {
        std::println("pf = ParsedFunds(s): v = {}, decimalPlaces = {}", pf->v, pf->decimalPlaces);
        auto test_funds { [pf](TokenPrecision p) {
            if (auto f { Funds_uint64::parse(*pf, p) }) {
                std::println("Funds_uint64(pf,{}) = {}", p.value(), f->val);
            } else {
                std::println("Funds_uint64(pf,{}): <no value> ", p.value());
            }
        } };
        test_funds(0);
        test_funds(4);
        test_funds(12);
        test_funds(16);
    } else {
        std::println("ParsedFunds: <no value>");
    }
}

int main()
{
    print_results("1.123");
    print_results("101.123000");
    print_results("101.1230001111");
    print_results("101.00000000000000");
    print_results("123123101.001");
    return 0;
}
