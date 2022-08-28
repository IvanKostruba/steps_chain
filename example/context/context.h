#pragma once

#include "../steps/possible_revert.h"
#include "../steps/sanctions_screening.h"
#include "../steps/start_transfer.h"
#include "../steps/unload_account.h"
#include "../api/api_mock.h"
#include "../db/db_mock.h"
#include "../timer/timer_mock.h"

#include <memory>

// This class purpose is to serve as an adapter between each step's interface and real I/O interface
// so this class should be devoid of business logic and only translate one call to another.

class PayoutContext : public PossibleRevertCtxI, public SanctionsScreeningCtxI,
	public StartTransferCtxI, public UnloadAccountCtxI
{
public:
	PayoutContext(
		std::shared_ptr<ApiMock> api,
		std::shared_ptr<DbMock> db,
		std::shared_ptr<TimerMock> timer
	);

	bool loadAccount(int consumerId, int amount) override;
	int transactionAmount(int transactionId) override;

	std::string beneficiaryName(int transactionId) override;
	bool passScreening(const std::string& screeningData) override;

	TransactionInfo transactionInfo(int transactionId) override;
	std::string consumerName(int consumerId) override;
	std::optional<std::string> initiateTransfer(
		const std::string& senderName, const TransactionInfo& info) override;
	void scheduleRetry(const std::string& requestId, int delay) override;
	void updateTransaction(int transactionId, const std::string& remoteId) override;

	int createTransaction(
		const std::string& requestId,
		int amount,
		const std::string& beneficiaryAccount,
		const std::string& beneficiaryName
	) override;
	bool unloadAccount(int consumerId, int amount) override;

private:
	std::shared_ptr<ApiMock> _api;
	std::shared_ptr<DbMock> _db;
	std::shared_ptr<TimerMock> _timer;
};
