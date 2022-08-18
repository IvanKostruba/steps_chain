#pragma once

#include "util.h"

#include <string>

// ---------- Testing is_serializable on invalid types ----------

struct InvalidParameterWrongSerialize {
    InvalidParameterWrongSerialize(std::string&& s) {}
    int serialize() { return 42; }
};

static_assert(!steps_chain::helpers::is_serializable<InvalidParameterWrongSerialize>::value,
    "InvalidParameterWrongSerialize must be detected as invalid!");

struct InvalidParameterNoStringCtor {
    std::string serialize() const { return ""; }
};

static_assert(!steps_chain::helpers::is_serializable<InvalidParameterNoStringCtor>::value,
    "InvalidParameterNoStringCtor must be detected as invalid!");