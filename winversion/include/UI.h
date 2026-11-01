#ifndef UI_H
#define UI_H
#include "FamilyNode.h"
#include "Auth.h"
#include "Registry.h"
#include <string>
void RunMenu(FamilyTree& familyTree, const std::string& familyPath,
             AccountList& accounts, AchievementList& achievements, RuleList& rules);
void PrintFamilyTree(Node* root);
#endif
