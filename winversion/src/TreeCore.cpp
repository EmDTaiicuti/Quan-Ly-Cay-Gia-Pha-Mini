#include "TreeCore.h"
#include <cctype>

namespace {
bool IsLeapYear(int year) { return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0); }

bool IsUniqueId(Node* node, int id) {
    if (!node) return true;
    if (node->member.id == id) return false;
    return IsUniqueId(node->firstChild, id) && IsUniqueId(node->nextSibling, id);
}

int CountBranch(Node* node) {
    if (!node) return 0;
    int count = 1;
    for (Node* child = node->firstChild; child; child = child->nextSibling)
        count += CountBranch(child);
    return count;
}

void ClearBranchSpouseLinks(Node* branch, Node* wholeTree) {
    if (!branch) return;
    if (branch->member.spouseId != 0) {
        Node* spouse = FindMember(wholeTree, branch->member.spouseId);
        if (spouse) spouse->member.spouseId = 0;
    }
    for (Node* child = branch->firstChild; child; child = child->nextSibling)
        ClearBranchSpouseLinks(child, wholeTree);
}

void AppendNode(Node*& head, Node*& tail, Node* added, Node* parent) {
    added->parent = parent;
    if (!head) head = added;
    else tail->nextSibling = added;
    tail = added;
}

int MaxId(Node* node) {
    if (!node) return 0;
    int best = node->member.id;
    int left = MaxId(node->firstChild);
    int right = MaxId(node->nextSibling);
    if (left > best) best = left;
    if (right > best) best = right;
    return best;
}

bool DeathDateOk(const Member& m) {
    return m.deathDate.empty() || (IsValidDate(m.deathDate) && CompareDates(m.deathDate, m.birthDate) >= 0);
}

// Gỡ một node (cùng cây con của nó) khỏi danh sách anh chị em rồi giải phóng
bool Unlink(FamilyTree& familyTree, Node* target) {
    Node* parent = target->parent;
    Node*& head = parent ? parent->firstChild : familyTree.root;
    Node*& tail = parent ? parent->lastChild : familyTree.lastRoot;
    Node* previous = 0;
    Node* current = head;
    while (current && current != target) {
        previous = current;
        current = current->nextSibling;
    }
    if (!current) return false;
    if (previous) previous->nextSibling = target->nextSibling;
    else head = target->nextSibling;
    if (tail == target) tail = previous;
    familyTree.memberCount -= CountBranch(target);
    target->nextSibling = 0;
    DestroyTree(target);
    return true;
}

void ClearDanglingSpouses(Node* node, Node* root) {
    if (!node) return;
    if (node->member.spouseId != 0) {
        Node* spouse = FindMember(root, node->member.spouseId);
        if (!spouse || spouse == node || spouse->member.spouseId != node->member.id) node->member.spouseId = 0;
    }
    ClearDanglingSpouses(node->firstChild, root);
    ClearDanglingSpouses(node->nextSibling, root);
}

void RepairInLawFlags(Node* node, Node* root) {
    if (!node) return;
    Node* spouse = node->member.spouseId ? FindMember(root, node->member.spouseId) : 0;
    if (!spouse) {
        // Dâu/rể bắt buộc phải có vợ/chồng trong phả đồ
        node->member.isInLaw = false;
    } else if (node->member.isInLaw && spouse->member.isInLaw) {
        // File cũ (7 cột) đánh dấu cả hai vợ chồng là dâu/rể: người được thêm sau (ID lớn hơn) mới là dâu/rể
        if (spouse->parent != node->parent) {
            node->member.isInLaw = false;
            spouse->member.isInLaw = false;
        } else if (node->member.id < spouse->member.id) {
            node->member.isInLaw = false;
        } else {
            spouse->member.isInLaw = false;
        }
    }
    RepairInLawFlags(node->firstChild, root);
    RepairInLawFlags(node->nextSibling, root);
}
}

void InitializeTree(FamilyTree& familyTree) {
    familyTree.root = 0;
    familyTree.lastRoot = 0;
    familyTree.memberCount = 0;
}

void DestroyTree(Node*& node) {
    if (!node) return;
    DestroyTree(node->firstChild);
    DestroyTree(node->nextSibling);
    delete node;
    node = 0;
}

Node* FindMember(Node* node, int id) {
    if (!node) return 0;
    if (node->member.id == id) return node;
    Node* found = FindMember(node->firstChild, id);
    return found ? found : FindMember(node->nextSibling, id);
}

Node* FindParent(FamilyTree& familyTree, int childId) {
    Node* member = FindMember(familyTree.root, childId);
    return member ? member->parent : 0;
}

bool IsValidDate(const std::string& date) {
    if (date.size() != 10 || date[2] != '/' || date[5] != '/') return false;
    for (std::size_t i = 0; i < date.size(); ++i)
        if (i != 2 && i != 5 && !std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    int day = (date[0] - '0') * 10 + date[1] - '0';
    int month = (date[3] - '0') * 10 + date[4] - '0';
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + date[9] - '0';
    if (year < 1 || month < 1 || month > 12) return false;
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = days[month - 1] + (month == 2 && IsLeapYear(year) ? 1 : 0);
    return day >= 1 && day <= maxDay;
}

int CompareDates(const std::string& firstDate, const std::string& secondDate) {
    int firstYear = (firstDate[6] - '0') * 1000 + (firstDate[7] - '0') * 100 + (firstDate[8] - '0') * 10 + firstDate[9] - '0';
    int secondYear = (secondDate[6] - '0') * 1000 + (secondDate[7] - '0') * 100 + (secondDate[8] - '0') * 10 + secondDate[9] - '0';
    if (firstYear != secondYear) return firstYear < secondYear ? -1 : 1;
    int firstMonth = (firstDate[3] - '0') * 10 + firstDate[4] - '0';
    int secondMonth = (secondDate[3] - '0') * 10 + secondDate[4] - '0';
    if (firstMonth != secondMonth) return firstMonth < secondMonth ? -1 : 1;
    int firstDay = (firstDate[0] - '0') * 10 + firstDate[1] - '0';
    int secondDay = (secondDate[0] - '0') * 10 + secondDate[1] - '0';
    if (firstDay == secondDay) return 0;
    return firstDay < secondDay ? -1 : 1;
}

std::string NormalizeDate(const std::string& date) {
    if (IsValidDate(date)) return date;
    if (date.size() == 10 && date[4] == '-' && date[7] == '-') {
        std::string converted = date.substr(8, 2) + "/" + date.substr(5, 2) + "/" + date.substr(0, 4);
        if (IsValidDate(converted)) return converted;
    }
    // Cho phép nhập thiếu số 0: 1/2/1970 -> 01/02/1970
    std::size_t a = date.find('/');
    std::size_t b = a == std::string::npos ? a : date.find('/', a + 1);
    if (b != std::string::npos && a >= 1 && a <= 2 && b - a - 1 >= 1 && b - a - 1 <= 2 && date.size() - b - 1 == 4) {
        std::string d = date.substr(0, a), m = date.substr(a + 1, b - a - 1);
        std::string converted = (d.size() == 1 ? "0" + d : d) + "/" + (m.size() == 1 ? "0" + m : m) + "/" + date.substr(b + 1);
        if (IsValidDate(converted)) return converted;
    }
    return date;
}

bool IsBirthDateValidForParent(const std::string& childDate, const std::string& parentDate) {
    return IsValidDate(childDate) && IsValidDate(parentDate) && CompareDates(childDate, parentDate) > 0;
}

bool AddRoot(FamilyTree& familyTree, const Member& member) {
    if (member.id <= 0 || !IsUniqueId(familyTree.root, member.id) || !IsValidDate(member.birthDate) || !DeathDateOk(member))
        return false;
    AppendNode(familyTree.root, familyTree.lastRoot, new Node(member), 0);
    ++familyTree.memberCount;
    return true;
}

bool AddChild(FamilyTree& familyTree, int parentId, const Member& member, ParentDatePolicy dates) {
    Node* parent = FindMember(familyTree.root, parentId);
    if (!parent || member.id <= 0 || !IsUniqueId(familyTree.root, member.id) || !IsValidDate(member.birthDate) ||
        !DeathDateOk(member))
        return false;
    // Dâu/rể được lưu chung nhóm với vợ/chồng nên "cha" của họ là cha/mẹ chồng (vợ): không so ngày sinh
    if (dates == ParentDatePolicy::Validate && !member.isInLaw &&
        !IsBirthDateValidForParent(member.birthDate, parent->member.birthDate)) return false;
    AppendNode(parent->firstChild, parent->lastChild, new Node(member, parent), parent);
    ++familyTree.memberCount;
    return true;
}

bool AddSpouse(FamilyTree& familyTree, int memberId, Member spouse) {
    Node* member = FindMember(familyTree.root, memberId);
    if (!member || member->member.spouseId != 0 || spouse.spouseId != 0 || spouse.id <= 0 ||
        !IsUniqueId(familyTree.root, spouse.id) || !IsValidDate(spouse.birthDate) || !DeathDateOk(spouse))
        return false;
    spouse.spouseId = memberId;
    spouse.isInLaw = true;
    member->member.spouseId = spouse.id;
    Node* parent = member->parent;
    if (parent) AppendNode(parent->firstChild, parent->lastChild, new Node(spouse, parent), parent);
    else AppendNode(familyTree.root, familyTree.lastRoot, new Node(spouse), 0);
    ++familyTree.memberCount;
    return true;
}

bool UpdateMember(FamilyTree& familyTree, int id, const Member& updatedMember) {
    Node* node = FindMember(familyTree.root, id);
    if (!node || !IsValidDate(updatedMember.birthDate) || !DeathDateOk(updatedMember)) return false;
    if (node->parent && !node->member.isInLaw &&
        !IsBirthDateValidForParent(updatedMember.birthDate, node->parent->member.birthDate))
        return false;
    for (Node* child = node->firstChild; child; child = child->nextSibling)
        if (!child->member.isInLaw && !IsBirthDateValidForParent(child->member.birthDate, updatedMember.birthDate))
            return false;
    const int originalId = node->member.id;
    const int originalSpouseId = node->member.spouseId;
    const bool originalInLaw = node->member.isInLaw;
    node->member = updatedMember;
    node->member.id = originalId;
    node->member.spouseId = originalSpouseId;
    node->member.isInLaw = originalInLaw;
    return true;
}

bool DeleteBranch(FamilyTree& familyTree, int id) {
    Node* target = FindMember(familyTree.root, id);
    if (!target) return false;
    // Xóa người huyết thống thì xóa luôn vợ/chồng (dâu/rể) đứng cạnh.
    // Xóa dâu/rể thì KHÔNG được xóa người huyết thống và con cháu của họ.
    Node* spouse = target->member.spouseId ? FindMember(familyTree.root, target->member.spouseId) : 0;
    if (spouse && spouse != target && !target->member.isInLaw && spouse->member.isInLaw &&
        spouse->parent == target->parent) {
        ClearBranchSpouseLinks(spouse, familyTree.root);
        Unlink(familyTree, spouse);
    }
    ClearBranchSpouseLinks(target, familyTree.root);
    return Unlink(familyTree, target);
}

int NextMemberId(Node* node) { return MaxId(node) + 1; }

int GetGeneration(Node* node) {
    int depth = 0;
    while (node) {
        ++depth;
        node = node->parent;
    }
    return depth;
}

int CountSubtree(Node* node) { return CountBranch(node); }

void ClearMarks(Node* node) {
    if (!node) return;
    node->mark = MARK_NONE;
    ClearMarks(node->firstChild);
    ClearMarks(node->nextSibling);
}

void MarkAncestors(Node* node) {
    for (Node* cur = node; cur; cur = cur->parent)
        if (cur->mark == MARK_NONE) cur->mark = MARK_ANCESTOR;
}

void MarkDescendants(Node* node, Node* treeRoot) {
    if (!node) return;
    if (node->mark == MARK_NONE) node->mark = MARK_DESCENDANT;
    for (Node* child = node->firstChild; child; child = child->nextSibling)
        MarkDescendants(child, treeRoot);
    if (!treeRoot || node->member.spouseId == 0) return;
    Node* spouse = FindMember(treeRoot, node->member.spouseId);
    if (!spouse) return;
    for (Node* child = spouse->firstChild; child; child = child->nextSibling)
        MarkDescendants(child, treeRoot);
}

void RepairFamilyLinks(FamilyTree& familyTree) {
    ClearDanglingSpouses(familyTree.root, familyTree.root);
    RepairInLawFlags(familyTree.root, familyTree.root);
}
