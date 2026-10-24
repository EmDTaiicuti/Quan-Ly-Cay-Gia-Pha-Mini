#include "FamilyNode.h"
#include "TreeCore.h"
#include "FileManager.h"
#include "UI.h"
#include "Auth.h"
#include "Registry.h"
#include <iostream>

int main() {
    FamilyTree familyTree; InitializeTree(familyTree);
    AccountList accounts; AchievementList achievements; RuleList rules;
    const std::string familyPath = "family.csv";
    bool loaded = LoadFromCsv(familyPath, familyTree) && LoadAccounts("accounts.csv", accounts) &&
                  LoadAchievements("achievements.csv", achievements) && LoadRules("family_rules.csv", rules);
    if (!loaded) {
        std::cerr << "Khong the mo/tao mot trong cac file du lieu.\n";
        DestroyTree(familyTree.root); DestroyAccounts(accounts); DestroyAchievements(achievements); DestroyRules(rules); return 1;
    }
    RunMenu(familyTree, familyPath, accounts, achievements, rules);
    bool saved = SaveToCsv(familyPath, familyTree) && SaveAccounts("accounts.csv", accounts) &&
                 SaveAchievements("achievements.csv", achievements) && SaveRules("family_rules.csv", rules);
    if (!saved) std::cerr << "Canh bao: khong the luu day du du lieu.\n";
    DestroyTree(familyTree.root); DestroyAccounts(accounts); DestroyAchievements(achievements); DestroyRules(rules);
    return saved ? 0 : 1;
}
