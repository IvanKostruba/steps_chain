#include "chain_wrapper.h"
#include "steps_chain.h"
#include "context_steps_chain.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace steps_chain;

// ---------- Testing/Usage example ----------

// A struct, conforming to basic type requirements for argument and return value

struct EmptyParameter {
    EmptyParameter() {}
    explicit EmptyParameter(const std::string &str) {}
    std::string serialize() const { return "EmptyParameter"; }
};
static_assert(helpers::is_serializable<EmptyParameter>::value,
              "EmptyParameter must be valid!");

struct IntParameter {
    explicit IntParameter(int v) : _value{v} {}
    explicit IntParameter(std::string&& s) { _value = std::stoi(s); }
    std::string serialize() const { return std::to_string(_value); }

    int _value;
};
static_assert(helpers::is_serializable<IntParameter>::value,
              "IntParameter must be valid!");

struct StringParameter {
    explicit StringParameter(std::string s) : _value{std::move(s)} {}
    std::string serialize() const { return _value; }

    std::string _value;
};
static_assert(helpers::is_serializable<StringParameter>::value,
              "StringParameter must be valid!");

// ---------- Test functions that just do output ----------

EmptyParameter boo(const EmptyParameter &) {
    std::cout << "  # Step output: boo( EmptyParameter ) -> EmptyParameter\n";
    return EmptyParameter{};
}

EmptyParameter goo(EmptyParameter) {
    std::cout << "  # Step output: goo( EmptyParameter ) -> EmptyParameter\n";
    return EmptyParameter{};
}
EmptyParameter goo_throw(const EmptyParameter &) {
    static size_t flag = 0;
    if (++flag == 1) {
        std::cout << "  # Step throws!!\n";
        throw(1);
    }
    std::cout <<  "  # Retry successful!! Step output: goo( EmptyParameter ) -> EmptyParameter\n";
    return EmptyParameter{};
}

auto hoo(EmptyParameter &) {
    std::cout << "  # Step output: hoo( EmptyParameter ) -> IntParameter{ 11 }\n";
    return IntParameter{11};
}

// ---------- Test class ----------

class TestClass {
public:
    explicit TestClass(int val) : _val{val} {}

    // Will only compile if you wrap it to lambda.
    EmptyParameter method(IntParameter& param) const {
        std::cout << "  # Step output: class method( IntParameter{ "
            << param._value << " } ) -> EmptyParameter\n";
        return EmptyParameter{""};
    }

    static IntParameter stat_method(EmptyParameter&) {
        std::cout <<
            "  # Step output: class static_method( EmptyParameter ) -> IntParameter{ 88 }\n";
        return IntParameter{88};
    }

    IntParameter operator()(const EmptyParameter&) const {
        std::cout <<
            "  # Step output: class instance [" << _val
            << "] operator()( EmptyParameter ) -> IntParameter{ 1024 }\n";
        return IntParameter{1024};
    }

private:
    int _val;
};

// ---------- Testing is_serializable on invalid types ----------

struct InvalidParameterWrongSerialize {
    InvalidParameterWrongSerialize(std::string &&s) {}
    int serialize() { return 42; }
};

static_assert(!helpers::is_serializable<InvalidParameterWrongSerialize>::value,
              "InvalidParameterWrongSerialize must be detected as invalid!");

struct InvalidParameterNoStringCtor {
    std::string serialize() const { return ""; }
};
static_assert(!helpers::is_serializable<InvalidParameterNoStringCtor>::value,
              "InvalidParameterNoStringCtor must be detected as invalid!");

// ---------- Testing helpers::are_chainable  ----------

static_assert(helpers::are_chainable<decltype(boo)>(), "Single-element chain should be always valid!");
static_assert(!helpers::are_chainable<decltype(hoo), decltype(boo)>(), "Mismatched signatures must be detected!");

// ---------- Output helper ----------

template <typename S>
void print_status(S& sequence) {
    auto [step, state] = sequence.get_current_state();
    std::cout << "  - Current step: " << step << ", finished: "
        << (sequence.is_finished() ? "yes" : "no") << ", step argument = " << state << "\n";
}

// ---------- Main ----------

int main(int argc, char **argv) {
    auto steps_with_fail = StepsChain{
        boo,
        goo_throw,
        hoo};
    std::cout << "Running flawed steps:\n";
    try {
        steps_with_fail.initialize("");
        while (!steps_with_fail.is_finished()) {
            steps_with_fail.advance();
            print_status(steps_with_fail);
        }
    }
    catch (int) {
        std::cout << "State after the exception:\n";
        print_status(steps_with_fail);
        std::cout << "Trying to resume...\n";
        steps_with_fail.resume();
        std::cout << "State after resume:\n";
        print_status(steps_with_fail);
    }

    auto steps = StepsChain{
        boo,
        goo,
        hoo,
        [](IntParameter &p) -> StringParameter {
            std::cout << "  # Step output: anon lambda( IntParameter{ "
                << p._value << " } ) -> StringParameter { ... }\n";
            return StringParameter{"Calculation result: " + std::to_string(p._value * 3)};
        }
    };
    std::cout << "\nRunning steps from custom point:\n";
    steps.run("", 1);
    print_status(steps);

    auto single_step = StepsChain {
        hoo
    };
    std::cout << "\nRunning single-step sequence:\n";
    single_step.run("");
    print_status(single_step);

    // Storing step chains in a container with the help of ChainWrapper
    std::unordered_map<std::string, ChainWrapper> chains;
    TestClass instance{1};
    auto class_wrapper = [&instance](IntParameter& param){ return instance.method(param); };
    chains.insert( { "chain_1", ChainWrapper{ StepsChain {
        TestClass::stat_method,
        //&TestClass::method would not compile, because we are not storing the pointer to class.
        class_wrapper,
        instance  // An instance of a functor can be passed though.
    }}});
    TestClass instance2{2};
    chains.insert( { "chain_2", ChainWrapper{ StepsChain {
        TestClass::stat_method,
        class_wrapper, // intentionally using wrapper for the previous instance
        instance2  // An instance of a functor can be passed though.
    }}});
    auto wrapperCopy = chains["chain_1"];

    std::cout << "\nRunning sequence [chain_1] with class methods:\n";
    chains["chain_1"].initialize("", 0);
    while (!chains["chain_1"].is_finished())
    {
        chains["chain_1"].advance();
        print_status(chains["chain_1"]);
    }

    // Here we are checking, that correct functor instance is used even though StepsChain type
    // is the same.
    std::cout << "\nRunning sequence [chain_2] with class methods with different functor instance:\n";
    chains["chain_2"].initialize("");
    chains["chain_2"].advance();
    print_status(chains["chain_2"]);
    chains["chain_2"].resume();
    print_status(chains["chain_2"]);

    std::cout << "\nChecking the copy we made before from [chain_1]:\n";
    print_status(wrapperCopy);

    std::cout << "\nTest wrapper with external context.\n";
    auto ctx_chain = ContextStepsChain{
        [](EmptyParameter& p, int ctx) -> IntParameter {
            std::cout << "Lambda with context [" << ctx << "] ( EmptyParameter )\n";
            return IntParameter{33};
        },
        [](IntParameter& p, int ctx) -> EmptyParameter {
            std::cout << "Lambda with context [" << ctx
                << "] ( IntParameter{ " << p._value << " } )\n";
            return EmptyParameter{};
        }
    };
    ctx_chain.initialize("");
    ctx_chain.advance(88);
    ctx_chain.resume(99);
    print_status(ctx_chain);
}
