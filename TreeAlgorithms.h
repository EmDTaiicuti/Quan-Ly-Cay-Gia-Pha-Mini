#ifndef TREEALGORITHMS_H
#define TREEALGORITHMS_H

#include <string>
#include "FamilyNode.h"

struct TreeStats {
    int total;         // tổng thành viên (gồm vợ/chồng nhập gia)
    int male;
    int female;
    int generations;   // số thế hệ của dòng họ
};

// ---- Duyệt cây----
bool DfsPrintLineage(FamilyNode* node, int targetId);                 // xuất xứ dòng họ bằng DFS
int  DfsSearchByName(FamilyNode* node, const std::string& keyword);   // tìm theo tên bằng DFS
void BfsPrintByGeneration(FamilyNode* root);                          // in từng thế hệ bằng BFS

// ---- Tổ tiên chung & khoảng cách ----
int GetDepth(const FamilyNode* node);                                 // gốc = 0
FamilyNode* FindLca(FamilyNode* a, FamilyNode* b);
int FamilyDistance(FamilyNode* a, FamilyNode* b);

// ---- Sắp xếp (Thành viên 5) ----
FamilyNode* SortSiblingList(FamilyNode* head);
void SortChildrenOf(FamilyNode* parent);
void SortAllSiblings(FamilyNode* node);

// ---- Thống kê (Thành viên 5) ----
int CountSubtree(const FamilyNode* node);
int TreeHeight(const FamilyNode* node);
TreeStats ComputeStats(const FamilyNode* root);

#endif // TREEALGORITHMS_H