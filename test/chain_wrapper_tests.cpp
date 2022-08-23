#include "parameters.h"
#include <steps_chain.h>
#include <context_steps_chain.h>
#include <chain_wrapper.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>


namespace {

IntParameter doubleValue(const IntParameter& data) {
    return IntParameter{ data._value * 2 };
}

auto make_chain() {
    return steps_chain::StepsChain{
        doubleValue,
        doubleValue,
        doubleValue
    };
}

};  // anonymous namespace

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

namespace {

enum class Element {
    SECOND,
    THIRD,
    FOURTH
};

steps_chain::ChainWrapper progressionFactory(Element p) {
    switch (p) {
    case Element::SECOND:
        return steps_chain::StepsChain{ doubleValue };
    case Element::THIRD:
        return steps_chain::StepsChain{ doubleValue, doubleValue };
    case Element::FOURTH:
        return steps_chain::StepsChain{
            doubleValue,
            doubleValue,
            [](const IntParameter& data){ return IntParameter{ data._value * 2 }; }
        };
    }
}

};  // anonymous namespace

// Step chains of different lengths or of different callables are different types.
TEST(ChainWrapperTests, WrappingDifferentChainTypes) {
    std::unordered_map<int, steps_chain::ChainWrapper> elem;
    elem[2] = progressionFactory(Element::SECOND);
    elem[3] = progressionFactory(Element::THIRD);
    elem[4] = progressionFactory(Element::FOURTH);
    elem[2].run("1");
    const auto [step_idx_after_0, data_after_0] = elem[2].get_current_state();
    ASSERT_EQ(step_idx_after_0, 1);
    ASSERT_EQ(data_after_0, "2");
    elem[3].run("3");
    const auto [step_idx_after_1, data_after_1] = elem[3].get_current_state();
    ASSERT_EQ(step_idx_after_1, 2);
    ASSERT_EQ(data_after_1, "12");
    elem[4].run("10");
    const auto [step_idx_after_2, data_after_2] = elem[4].get_current_state();
    ASSERT_EQ(step_idx_after_2, 3);
    ASSERT_EQ(data_after_2, "80");
    ASSERT_TRUE(elem[2].is_finished()
        && elem[3].is_finished()
        && elem[4].is_finished()
    );
}

namespace {

struct MockUserDB {
    explicit MockUserDB(int count) : _userCount{ count } {}

    int getUserCount() const { return _userCount; }

    int _userCount;
};

IntParameter usersCount(EmptyParameter p, const MockUserDB& db) {
    return IntParameter{ db.getUserCount() };
}

};  // anonymous namespace

// Even steps with and without context can be wrapped in the same manner and put into the same
// container. Each wrapped chain with context has an individual context instance.
TEST(ChainWrapperTests, WrappingChainsWithContext) {
    std::unordered_map<std::string, steps_chain::ChainWrapper> processes;
    processes["userCount_10"] = steps_chain::ChainWrapper{
        steps_chain::ContextStepsChain{usersCount}, MockUserDB{10} };
    processes["userCount_20"] = steps_chain::ChainWrapper{
        steps_chain::ContextStepsChain{usersCount}, MockUserDB{20} };
    processes["quadruple"] = steps_chain::ChainWrapper{
        steps_chain::StepsChain{ doubleValue, doubleValue } };
    processes["userCount_10"].run("");
    const auto [step_idx_after_0, data_after_0] = processes["userCount_10"].get_current_state();
    ASSERT_EQ(step_idx_after_0, 1);
    ASSERT_EQ(data_after_0, "10");
    processes["userCount_20"].run("");
    const auto [step_idx_after_1, data_after_1] = processes["userCount_20"].get_current_state();
    ASSERT_EQ(step_idx_after_1, 1);
    ASSERT_EQ(data_after_1, "20");
    processes["quadruple"].initialize("1");
    processes["quadruple"].resume();
    const auto [step_idx_after_2, data_after_2] = processes["quadruple"].get_current_state();
    ASSERT_EQ(step_idx_after_2, 2);
    ASSERT_EQ(data_after_2, "4");
    ASSERT_TRUE(processes["userCount_10"].is_finished()
        && processes["userCount_20"].is_finished()
        && processes["quadruple"].is_finished()
    );
}
