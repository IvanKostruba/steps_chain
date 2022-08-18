#include "parameters.h"
#include "context_steps_chain.h"

#include <optional>
#include <string>

#include <gtest/gtest.h>

class MockIoContext {
public:
	int sendHttpRequest(const std::string& url) const { return 200; };
	int makeRpcCall(int data) const { return data; };
};

IntParameter doubleRpcResult(const IntParameter& data, MockIoContext ctx) {
	const auto rpcResult = ctx.makeRpcCall(data._value);
	return IntParameter{ rpcResult * 2 };
}

IntParameter doubleRpcResultRef(const IntParameter& data, const MockIoContext& ctx) {
    const auto rpcResult = ctx.makeRpcCall(data._value);
    return IntParameter{ rpcResult * 2 };
}

static_assert(!steps_chain::helpers::are_chainable<
    decltype(doubleRpcResult), decltype(doubleRpcResultRef)>(),
    "Mismatched context types (val vs. ref) must be detected!");

TEST(RawContextChainTests, SingleStepChain) {
    auto single_step = steps_chain::ContextStepsChain{
        doubleRpcResult
    };
    const auto [step_idx_before, data_before] = single_step.get_current_state();
    ASSERT_EQ(step_idx_before, 0);
    ASSERT_EQ(data_before, "0");
    single_step.run("42", MockIoContext{});
    const auto [step_idx_after, data_after] = single_step.get_current_state();
    ASSERT_EQ(step_idx_after, 1);
    ASSERT_EQ(data_after, "84");
    ASSERT_TRUE(single_step.is_finished());
}

TEST(RawContextChainTests, MultipleStepsChain) {
    auto multiple_steps = steps_chain::ContextStepsChain {
        doubleRpcResult,
        doubleRpcResult,
        doubleRpcResult
    };
    const auto [step_idx_before, data_before] = multiple_steps.get_current_state();
    ASSERT_EQ(step_idx_before, 0);
    ASSERT_EQ(data_before, "0");
    multiple_steps.initialize("1");
    const auto [step_idx_init, data_init] = multiple_steps.get_current_state();
    ASSERT_EQ(step_idx_init, 0);
    ASSERT_EQ(data_init, "1");
    MockIoContext ioContext;
    while (!multiple_steps.is_finished()) {
        multiple_steps.advance(ioContext);
    }
    const auto [step_idx_after, data_after] = multiple_steps.get_current_state();
    ASSERT_EQ(step_idx_after, 3);
    ASSERT_EQ(data_after, "8");
}

struct EmptyContext {};

EmptyParameter foo(const EmptyParameter&, EmptyContext) {
    return EmptyParameter{};
}

EmptyParameter foo_throw_once(const EmptyParameter&, EmptyContext) {
    static size_t flag = 0;
    if (++flag == 1) {
        throw(1);
    }
    return EmptyParameter{};
}

auto foo_returns_int(EmptyParameter&, EmptyContext) {
    return IntParameter{ 11 };
}

EmptyParameter foo_context_by_ref(const EmptyParameter&, EmptyContext&) {
    return EmptyParameter{};
}

// ---------- Testing helpers::are_chainable  ----------

static_assert(steps_chain::helpers::are_chainable<decltype(foo)>(),
    "Single-element chain should be always valid!");
static_assert(!steps_chain::helpers::are_chainable<decltype(foo_returns_int), decltype(foo)>(),
    "Mismatched signatures must be detected!");
static_assert(!steps_chain::helpers::are_chainable<decltype(foo), decltype(foo_context_by_ref)>(),
    "Mismatched context types (val vs. ref) must be detected!");

// -----------------------------------------------------

TEST(RawContextChainTests, ResumeAfterException) {
    auto steps_with_fail = steps_chain::ContextStepsChain{
        foo,
        foo_throw_once,
        foo_returns_int
    };
    try {
        steps_with_fail.initialize("");
        while (!steps_with_fail.is_finished()) {
            steps_with_fail.advance(EmptyContext{});
        }
    }
    catch (int) {
        const auto [step_idx_exept, data_except] = steps_with_fail.get_current_state();
        ASSERT_EQ(step_idx_exept, 1);
        ASSERT_EQ(data_except, "EmptyParameter");
        steps_with_fail.resume(EmptyContext{});
    }
    ASSERT_TRUE(steps_with_fail.is_finished());
    const auto [step_idx_after, data_after] = steps_with_fail.get_current_state();
    ASSERT_EQ(step_idx_after, 3);
    ASSERT_EQ(data_after, "11");  // Hardcoded value from foo_returns_int
}

std::optional<EmptyParameter> interrupt(const EmptyParameter&, EmptyContext) {
    return std::nullopt;
}

TEST(RawContextChainTests, ChainInterruption) {
    auto steps_with_interrupt = steps_chain::ContextStepsChain{
        foo,
        interrupt,
        foo_returns_int
    };
    steps_with_interrupt.run("", EmptyContext{});
    ASSERT_FALSE(steps_with_interrupt.is_finished());
    const auto [step_idx_after, data_after] = steps_with_interrupt.get_current_state();
    ASSERT_EQ(step_idx_after, 1);
    ASSERT_EQ(data_after, "EmptyParameter");
}

TEST(RawContextChainTests, LambdasChain_StartFromMiddle) {
    auto lambdas_chain = steps_chain::ContextStepsChain{
        [](EmptyParameter p [[maybe_unused]], EmptyContext) { return IntParameter{42}; },
        [](const IntParameter& p, EmptyContext) { return IntParameter{ p._value + 10 }; },
        [](const IntParameter& p, EmptyContext) { EXPECT_EQ(p._value, 11); return EmptyParameter{}; }
    };
    lambdas_chain.initialize("1", 1);  // Start from the second step
    const auto [step_idx_init, data_init] = lambdas_chain.get_current_state();
    ASSERT_EQ(step_idx_init, 1);
    ASSERT_EQ(data_init, "1");
    lambdas_chain.resume(EmptyContext{});
    const auto [step_idx_after, data_after] = lambdas_chain.get_current_state();
    ASSERT_EQ(step_idx_after, 3);
    ASSERT_EQ(data_after, "EmptyParameter");
}

class ImmutableFunctor {
public:
    // Note that operator() is const
    IntParameter operator()(EmptyParameter p [[maybe_unused]], EmptyContext) const { return IntParameter{ 3 }; }
};

class Foo {
public:
    static IntParameter static_method(const IntParameter& p [[maybe_unused]], EmptyContext) { return IntParameter{ 0 }; }
};

TEST(RawContextChainTests, VariousCallablesInChain) {
    ImmutableFunctor immutableFunctorInstance;
    auto diverse_chain = steps_chain::ContextStepsChain{
        foo,
        // &Foo::normal_class_method -- won't compile, since we don't store a pointer to an instance.
        immutableFunctorInstance,
        [](IntParameter p, EmptyContext) { EXPECT_EQ(p._value, 3); return IntParameter{ p._value * 3}; },
        Foo::static_method
    };
    diverse_chain.run("", EmptyContext{});
    const auto [step_idx_after, data_after] = diverse_chain.get_current_state();
    ASSERT_EQ(step_idx_after, 4);
    ASSERT_EQ(data_after, "0");
}
