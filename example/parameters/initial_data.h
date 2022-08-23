#pragma once

#include <string>

struct InitialData {
	std::string requestId;
	int consumerId;
	int amount;
	std::string beneficiaryAccount;
	std::string beneficiaryName;

	InitialData() = default;
	explicit InitialData(const std::string& data);
	std::string serialize() const;
};
