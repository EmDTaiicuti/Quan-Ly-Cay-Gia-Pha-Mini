#ifndef TREE_ALGORITHMS_H
#define TREE_ALGORITHMS_H

#include "FamilyNode.h"
#include <iosfwd>
#include <string>

Node* FindByDfs(Node* node, int id);
void PrintDfs(Node* node, std::ostream& output, int depth);
void PrintBfsByGeneration(Node* root, std::ostream& output);
Node* FindLowestCommonAncestor(Node* root, int firstId, int secondId);
int GetRelationshipDistance(Node* root, int firstId, int secondId);
void SortSiblingsByBirthDate(Node*& firstSibling);
void CollectStatistics(Node* root, FamilyStats& stats);
void SearchByNameContains(Node* root, const std::string& query, std::ostream& output, int& count);
void CountYear(Node* root, int year, int& births, int& deaths);

#endif
