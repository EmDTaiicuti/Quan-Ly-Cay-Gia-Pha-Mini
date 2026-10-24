#ifndef TREE_CORE_H
#define TREE_CORE_H

#include "FamilyNode.h"

void InitializeTree(FamilyTree& familyTree);
void DestroyTree(Node*& node);
Node* FindMember(Node* node, int id);
Node* FindParent(FamilyTree& familyTree, int childId);
bool AddRoot(FamilyTree& familyTree, const Member& member);
bool AddChild(FamilyTree& familyTree, int parentId, const Member& member);
bool AddSpouse(FamilyTree& familyTree, int memberId, Member spouse);
bool UpdateMember(FamilyTree& familyTree, int id, const Member& updatedMember);
bool DeleteBranch(FamilyTree& familyTree, int id);
bool IsValidDate(const std::string& date);
int CompareDates(const std::string& firstDate, const std::string& secondDate);
std::string NormalizeDate(const std::string& date);
bool IsBirthDateValidForParent(const std::string& childDate, const std::string& parentDate);

#endif

