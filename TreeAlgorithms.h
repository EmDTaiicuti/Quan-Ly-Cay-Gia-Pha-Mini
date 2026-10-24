#ifndef TREEALGORITHMS_H
#define TREEALGORITHMS_H

#include <string>
#include "FamilyNode.h"

// Quy ước về vợ/chồng nhập gia (member.isInLaw = true):
//  - Không phải "con" của ai: không tham gia sắp xếp anh em,
//    không làm tăng số thế hệ, không bị duyệt như một nhánh con.
//  - Được ghép với người huyết thống bằng member.spouseId
//    (hai người là vợ/chồng nếu id của người này nằm ở spouseId của người kia
//    và họ cùng nằm trong một chuỗi anh em, đúng như dữ liệu family.csv).
//  - Vẫn được tính vào tổng thành viên và kết quả tìm kiếm.

// ---- Duyệt cây ----
bool DfsPrintLineage(Node* node, int targetId);                 // xuất xứ dòng họ bằng DFS (node = đầu chuỗi anh em)
int  DfsSearchByName(Node* node, const std::string& keyword);   // tìm theo tên bằng DFS (không phân biệt hoa/thường, có/không dấu)
void BfsPrintByGeneration(Node* root);                          // in từng thế hệ bằng BFS (root = đầu chuỗi gốc)

// ---- Tổ tiên chung & khoảng cách ----
int   GetDepth(const Node* node);                               // gốc = 0
Node* FindLca(Node* a, Node* b);
int   FamilyDistance(Node* a, Node* b);

// ---- Sắp xếp ----
Node* SortSiblingList(Node* head);       // chỉ sắp xếp con ruột; vợ/chồng đứng ngay sau người phối ngẫu
void  SortChildrenOf(Node* parent);      // sắp xếp con của parent và cập nhật firstChild + lastChild
void  SortAllSiblings(Node* node);       // sắp xếp toàn bộ nhánh gốc tại node

// ---- Thống kê ----
int         CountSubtree(const Node* node);
int         TreeHeight(const Node* node);
FamilyStats ComputeStats(const Node* root);   // root = đầu chuỗi gốc (có thể nhiều gốc)

#endif // TREEALGORITHMS_H