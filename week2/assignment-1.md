```cpp
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <algorithm>

#define THREAD_NUMBER 100
#define ACCOUNT_NUMBER 10

class TransBlock {
public:
    int id;
    int src_id;
    int dest_id;
    int amount;
    bool failure;
    std::chrono::steady_clock::time_point timestamp;

    TransBlock(int sid, int did, unsigned int amount, int identifier) {
        id = identifier;
        this->src_id = sid;
        this->dest_id = did;
        this->amount = amount;
        timestamp = std::chrono::steady_clock::now();
        failure = false;
    }
};

class Account {
public:
    int id;
    unsigned int balance;
    int last_check;
    int last_check_transaction_index;
    std::vector<TransBlock*> transactions;
    std::mutex m;

    Account(int id, unsigned int balance) {
        this->id = id;
        this->balance = balance;
        this->last_check = balance;
        this->last_check_transaction_index = 0;
    }

    void send(unsigned int amount, Account* acc, int identifier) {
        transactions.push_back(new TransBlock(id, acc->id, -amount, identifier));
    }

    void received(unsigned int amount, Account* acc, int identifier) {
        transactions.push_back(new TransBlock(acc->id, id, amount, identifier));
    }

    void drop(Account* src, Account* dest, int identifier) {
        std::vector<TransBlock*>::reverse_iterator rit;
        rit = src->transactions.rbegin();
        for (; rit!= src->transactions.rend(); ++rit) {
            if ((*rit)->id == identifier) {
                (*rit)->failure = true;
            }
        }
        rit = dest->transactions.rbegin();
        for (; rit!= dest->transactions.rend(); ++rit) {
            if ((*rit)->id == identifier) {
                (*rit)->failure = true;
            }
        }
    }

    std::vector<TransBlock*> get_transactions(std::chrono::steady_clock::time_point previous_to) {
        return this->transactions;
    }
};

bool transfer(Account* src, Account* dest, unsigned int amount, int identifier) {
    int success, threshold = 10;
    std::chrono::milliseconds timespan(rand() % 1000 + 1);
    do {
        threshold--;
        src->m.lock();
        if(src->balance >= amount) {
            src->balance -= amount;
            src->send(amount, dest, identifier);
        }
        success = dest->m.try_lock();
        if(success) {
            dest->balance += amount;
            dest->received(amount, src, identifier);
            dest->m.unlock();
            src->m.unlock();
            return true;
        } else {
            src->balance += amount;
            dest->received(amount, src, identifier);
            src->drop(src, dest, identifier);
            src->m.unlock();
            std::this_thread::sleep_for(timespan);
        }
    } while (success == 0 and threshold > 0);
    return false;
}

void display_transaction(TransBlock* transaction, std::chrono::steady_clock::time_point now) {
    std::cout << "transaction #" << transaction->id << " info: \n\tsrc: "
              << transaction->src_id << ", dest: "
              << transaction->dest_id << ", amount: "
              << transaction->amount << ", time ago: "
              << std::chrono::duration_cast<std::chrono::microseconds>(transaction->timestamp - now).count()
              << "us, failure status: " << transaction->failure << std::endl;
}

bool checker(std::vector<Account*> accounts, int identifier) {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::vector<std::vector<TransBlock*>> books;
    std::vector<int> balances;
    for(int iAcc = 0; iAcc < ACCOUNT_NUMBER; iAcc++) {
        accounts[iAcc]->m.lock();
        books.push_back(accounts[iAcc]->get_transactions(now));
        balances.push_back(accounts[iAcc]->balance);
        accounts[iAcc]->m.unlock();
        std::cout << "\nAccount " << iAcc << " log records:" << std::endl;
        for(auto &transaction : accounts[iAcc]->get_transactions(now)) {
            display_transaction(transaction, now);
        }
        std::cout << "balance: " << balances[iAcc] << "$" << std::endl;
    }

    std::cout << std::endl << "CHECKING... \n";
    std::vector<int> log_inconsistent_accounts;
    for(int iAcc = 0; iAcc < ACCOUNT_NUMBER; iAcc++) {
        for(auto &transaction : books[iAcc]) {
            bool log_inconsistency = false;
            for (auto &inner_transaction : books[transaction->dest_id]) {
                if (inner_transaction->id == transaction->id and inner_transaction->src_id == transaction->src_id
                    and inner_transaction->amount + transaction->amount == 0) {
                    log_inconsistency = true;
                }
            }

            if (!log_inconsistency and !books[transaction->dest_id].empty()) {
//                std::cout << "destination log inconsistency @transaction: " << transaction->id
//                    << ": operations not yet logged. " << std::endl;
                log_inconsistent_accounts.push_back(transaction->src_id);
                log_inconsistent_accounts.push_back(transaction->dest_id);
            }
        }
    }

    for(int iAcc = 0; iAcc < ACCOUNT_NUMBER; iAcc++) {
        int begin = accounts[iAcc]->last_check_transaction_index;
        for (int i = begin; i < books[iAcc].size(); i++) {
            if (!books[iAcc][i]->failure) {
                accounts[iAcc]->last_check += books[iAcc][i]->amount;
            }
        }

        accounts[iAcc]->last_check_transaction_index = books[iAcc].size();

        if (accounts[iAcc]->last_check != balances[iAcc] and
            std::find(std::begin(log_inconsistent_accounts), std::end(log_inconsistent_accounts), iAcc) ==
            std::end(log_inconsistent_accounts)) {
            std::cout << "Account " << iAcc << " invalid balance." << std::endl;
            return false;
        }
    }

    return true;
}

int main()
{
    srand(time(nullptr));
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::vector<Account*> accounts;
    std::vector<std::thread*> threads;

    for(int iAcc = 0; iAcc < ACCOUNT_NUMBER; iAcc++) {
        accounts.push_back(new Account(iAcc, rand() % 100 + 1));
        std::cout << "Account " << accounts.back()->id << ": " << accounts.back()->balance << "$" << std::endl;
    }

    std::cout << "\nBEGIN:" << std::endl;

    for(int iThrd = 0; iThrd < THREAD_NUMBER; iThrd++) {
        int i = 0, j = 0;
        while(i == j) {
            i = rand() % ACCOUNT_NUMBER;
            j = rand() % ACCOUNT_NUMBER;
        }
        if(i < 0) i = -i;
        if(j < 0) j = -j;

        threads.push_back(new std::thread(&transfer, accounts[i], accounts[j], rand() % 9 + 1, iThrd));

        if(iThrd % 10 == 0) {
            threads.push_back(new std::thread(checker, accounts, iThrd));
        }
    }

    for (int iThrd = 0; iThrd < THREAD_NUMBER + THREAD_NUMBER/10; iThrd++) {
        threads[iThrd]->join();
    }

    wait(nullptr);

    std::cout << "\nAFTER ONE RUN:\n";

    for (int iAcc = 0; iAcc < ACCOUNT_NUMBER; iAcc++) {
        std::cout << "Account " << accounts[iAcc]->id << ": " << accounts[iAcc]->balance << "$" << std::endl;
        for (auto &transaction : accounts[iAcc]->transactions) {
            std::cout << "\t#" << transaction->id << " ";
            if (transaction->failure) {
                std::cout << "failed to ";
            }
            if (transaction->amount < 0) {
                std::cout << "sent " << -transaction->amount << " to " << transaction->dest_id;
            } else {
                std::cout << "received " << transaction->amount << " from " << transaction->src_id;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "BENCHMARK: \nRunning took "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << "us.\n";

    return 0;
}
```
