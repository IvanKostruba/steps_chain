#include "initial_data.h"

InitialData::InitialData(const std::string& data) {
	requestId = data.substr(0, 8);
	consumerId = std::stoi(data.substr(9, 4));
	amount = std::stoi(data.substr(14, 4));
	std::string beneficiaryAccount = data.substr(15, 12);
	std::string beneficiaryName = data.substr(28);
}

std::string InitialData::serialize() const {
	return requestId + " "
		+ std::to_string(consumerId) + " "
		+ std::to_string(amount) + " "
		+ beneficiaryAccount + " " 
		+ beneficiaryName;
}
