# steps_chain

A library building block that allows functions, functors and lambdas be combined together in a sequence, so that previous function output is passed as the next function input. On each step, current arguments can be retrieved in serialized form. Also, sequence can be started (or resumed) from any step, either using current state, or initializing state from serialized argument.

See examples.cpp for usage examples.
