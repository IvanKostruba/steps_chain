#include "payout_process.h"
#include "api/api_mock.h"
#include "db/db_mock.h"
#include "timer/timer_mock.h"

#include <iostream>
#include <stdexcept>

void runProcess(
    steps_chain::ChainWrapper p,
    const std::string& requestId,
    std::shared_ptr<DbMock> db
) {
    const auto data = db->fetchProcessData(requestId);
    p.initialize(data.parameters, data.stepIdx);
    try {
        while (!p.is_finished()) {
            p.advance();
            const auto [idx, params] = p.get_current_state();
            db->setProcessData(data.requestId, idx, params);
        }
    }
    catch (const std::exception& ex) {
        std::cout << "Exception in processing of request [" << data.requestId << "]: "
            << ex.what() << "\n";
        return;
    }
    std::cout << "Processing of request [" << data.requestId
        << "] completed successfully.\n";
    std::cout << "Final processing data: " << db->fetchProcessData(requestId).parameters << "\n";
    std::cout << "Transaction created with id [" << db->_transactions.back().transactionId << "]\n";
}

int main(int argc, char **argv) {
    auto api = std::make_shared<ApiMock>();
    auto db = std::make_shared<DbMock>();
    auto timer = std::make_shared<TimerMock>();
    db->_consumers[1001] = "Franz Ferdinand";
    const std::string incomingRequest_1{"ABCD-101 1001 3241 DE0243983278 Thereza Mustermann"};
    db->setProcessData("ABCD-101", 0, incomingRequest_1);
    runProcess(payoutProcess(api, db, timer), "ABCD-101", db);
    return 1;
}
