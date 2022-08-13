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
