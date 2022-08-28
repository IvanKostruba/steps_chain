#include "payout_process.h"

#include "steps/possible_revert.h"
#include "steps/sanctions_screening.h"
#include "steps/start_transfer.h"
#include "steps/unload_account.h"
#include "context/context.h"

#include <context_steps_chain.h>

steps_chain::ChainWrapper payoutProcess(
	std::shared_ptr<ApiMock> api,
	std::shared_ptr<DbMock> db,
	std::shared_ptr<TimerMock> timer
) {
	// Steps are desined with the Interface Segregation principle in mind, so they accept different
	// types as their 'context'. So we have to wrap the steps in lambdas here, as ContextStepsChain
	// requires identical type of 'context' as a second argument of all steps.
	return steps_chain::ChainWrapper {
		steps_chain::ContextStepsChain{
			[](InitialData d,     std::shared_ptr<PayoutContext> c) { return unloadAccount(d, *c.get()); },
			[](TransactionData d, std::shared_ptr<PayoutContext> c) { return sanctionsScreening(d, *c.get()); },
			[](ComplianceData d,  std::shared_ptr<PayoutContext> c) { return possibleRevert(d, *c.get()); },
			[](TransactionData d, std::shared_ptr<PayoutContext> c) { return startTransfer(d, *c.get()); }
		},
		std::make_shared<PayoutContext>(api, db, timer)
	};
}
