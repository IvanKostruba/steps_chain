#pragma once

#include "util.h"

#include "static_assert_tests.h"

#include <string>

// All these parameter structs conforms to the requirements - they can be constructed from a string
// (we use different kinds of argument passing for testing) and have 'serialize' method that
// returns a string.

struct EmptyParameter {
    EmptyParameter() = default; // Argument of the first step must be default constructible.
    explicit EmptyParameter(const std::string& str [[maybe_unused]] ) {}
    std::string serialize() const { return "EmptyParameter"; }
};
static_assert(steps_chain::helpers::is_serializable<EmptyParameter>::value,
    "EmptyParameter must be valid!");

struct IntParameter {
    IntParameter() : _value{ 0 } {}
    explicit IntParameter(int v) : _value{ v } {}
    explicit IntParameter(std::string&& s) { _value = std::stoi(s); }
    std::string serialize() const { return std::to_string(_value); }

    int _value;
};
static_assert(steps_chain::helpers::is_serializable<IntParameter>::value,
    "IntParameter must be valid!");

struct TwoIntParameter {
    TwoIntParameter() : _a{ 1 }, _b{ 1 } {}
    TwoIntParameter(int a, int b) : _a{ a }, _b{ b } {}
    explicit TwoIntParameter(std::string s) { _b = _a = std::stoi(s); }
    // serialize method is deliberately incorrect to make parsing trivial
    std::string serialize() const { return std::to_string(_a); }

    int _a;
    int _b;
};

static_assert(steps_chain::helpers::is_serializable<TwoIntParameter>::value,
    "TwoIntParameter must be valid!");
