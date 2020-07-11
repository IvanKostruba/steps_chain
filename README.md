# steps_chain

include/steps_chain.h: A library building block that allows functions, functors and lambdas to be combined together in a sequence, so that previous function output is passed as the next function input. On each step, current argument can be retrieved in serialized form. Also, sequence can be started (or resumed) from any step, either using current state, or initializing state from serialized argument.

include/chain_wrapper.h: Wrapper, providing polymorphic behaviour for StepsChain, allowing to store chains in STL containers.

See examples.cpp for usage examples.
