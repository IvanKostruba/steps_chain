#include "parameters.h"
#include "steps_chain.h"
#include "context_steps_chain.h"
#include "chain_wrapper.h"

#include <optional>
#include <vector>

#include <gtest/gtest.h>


namespace {

IntParameter doubleValue(const IntParameter& data) {
    return IntParameter{ data._value * 2 };
}

} // anonymous namespace

auto make_chain() {
    return steps_chain::StepsChain{
        doubleValue,
        doubleValue,
        doubleValue
    };
}

TEST(ChainWrapperTests, IndependentState) {
    auto operationSequences = std::vector<steps_chain::ChainWrapper>{ make_chain(), make_chain() };
    operationSequences.push_back(make_chain());
    operationSequences[0].initialize("1", 1);  // Start from the second step
    const auto [step_idx_init_0, data_init_0] = operationSequences[0].get_current_state();
    ASSERT_EQ(step_idx_init_0, 1);
    ASSERT_EQ(data_init_0, "1");
    while (!operationSequences[0].is_finished()) {
        operationSequences[0].advance();
    }
    const auto [step_idx_after_0, data_after_0] = operationSequences[0].get_current_state();
    ASSERT_EQ(step_idx_after_0, 3);
    ASSERT_EQ(data_after_0, "4");
    operationSequences[1].run("1");  // Run all steps from the beginning
    const auto [step_idx_after_1, data_after_1] = operationSequences[1].get_current_state();
    ASSERT_EQ(step_idx_after_1, 3);
    ASSERT_EQ(data_after_1, "8");
    const auto [step_idx_init_2, data_init_2] = operationSequences[2].get_current_state();
    ASSERT_EQ(step_idx_init_2, 0);  // Third chain remained uninitialized and did not run
    ASSERT_EQ(data_init_2, "0");
    ASSERT_TRUE(operationSequences[0].is_finished()
        && operationSequences[1].is_finished()
        && !operationSequences[2].is_finished()
    );
}
