add_executable(
	runExample
	"example.cpp"
	"payout_process.cpp"
	"parameters/initial_data.cpp"
	"parameters/transaction_data.cpp"
	"parameters/compliance_data.cpp"
	"context/context.cpp"
	"steps/unload_account.cpp"
	"steps/sanctions_screening.cpp"
	"steps/possible_revert.cpp"
	"steps/start_transfer.cpp"
	"db/db_mock.cpp"
	"api/api_mock.cpp" "timer/timer_mock.cpp")
target_link_libraries(runExample PRIVATE steps_chain)