#ifndef AUTH_H
#define AUTH_H

#include <string>

struct Account {
    std::string username;
    std::string email;
    std::string salt;
    std::string passwordHash;
    int memberId;
    char role;       // A = admin, U = user.
    bool active;
    Account* next;
    Account() : username(), email(), salt(), passwordHash(), memberId(0), role('U'), active(true), next(0) {}
};

struct AccountList {
    Account* head;
    Account* tail;
    AccountList() : head(0), tail(0) {}
};

void DestroyAccounts(AccountList& accounts);
bool LoadAccounts(const std::string& path, AccountList& accounts);
bool SaveAccounts(const std::string& path, const AccountList& accounts);
bool HasAdminAccount(const AccountList& accounts);
Account* FindAccount(const AccountList& accounts, const std::string& username);
Account* Authenticate(AccountList& accounts, const std::string& username, const std::string& password);
bool CreateAccount(AccountList& accounts, const std::string& username, const std::string& email,
                   const std::string& password, int memberId, char role);
bool DeactivateAccount(AccountList& accounts, const std::string& username);

#endif
