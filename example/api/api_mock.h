#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ApiMock {
public:
	bool loadConsumerAccount(int consumerId, int amount);
	bool passScreening(const std::string& screeningData);
	bool unloadConsumerAccount(int consumerId, int amount);
	std::optional<std::string> initiateTransfer(
		const std::string& senderName,
		int amount,
		const std::string& beneficiaryAccount,
		const std::string& beneficiaryName
	);

	std::unordered_map<int, int> _balance;
	std::unordered_set<std::string> _sanctions;
	std::vector<std::string> _transfers;
	std::unordered_set<std::string> _errors;
};
