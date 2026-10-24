#include "TreeAlgorithms.h"
#include "TreeCore.h"
#include <ostream>

namespace {
struct QueueNode {
    Node* treeNode;  // NĂºt gia pháº£ Ä‘ang chá» xá»­ lĂ½.
    int depth;       // Tháº¿ há»‡ cá»§a nĂºt trong lÆ°á»£t BFS.
    QueueNode* next; // Pháº§n tá»­ káº¿ tiáº¿p trong hĂ ng Ä‘á»£i.
    QueueNode(Node* value, int level) : treeNode(value), depth(level), next(0) {}
};
struct Queue {
    QueueNode* front; // Äáº§u hĂ ng Ä‘á»£i, nÆ¡i láº¥y pháº§n tá»­ ra.
    QueueNode* back;  // Cuá»‘i hĂ ng Ä‘á»£i, nÆ¡i thĂªm pháº§n tá»­ vĂ o.
    Queue() : front(0), back(0) {}
    void Push(Node* node, int depth) {
        QueueNode* item = new QueueNode(node, depth);
        if (!back) front = back = item; else { back->next = item; back = item; }
    }
    QueueNode* Pop() {
        if (!front) return 0;
        QueueNode* item = front; front = front->next;
        if (!front) back = 0;
        item->next = 0; return item;
    }
    ~Queue() { while (front) { QueueNode* item = front; front = front->next; delete item; } }
};
void CountRecursive(Node* node, int depth, FamilyStats& stats) {
    if (!node) return;
    ++stats.totalMembers;
    if (node->member.gender == 'M') ++stats.maleCount;
    else if (node->member.gender == 'F') ++stats.femaleCount;
    else ++stats.otherCount;
    if (depth > stats.generationCount) stats.generationCount = depth;
    CountRecursive(node->firstChild, depth + 1, stats);
    CountRecursive(node->nextSibling, depth, stats);
}
}

Node* FindByDfs(Node* node, int id) {
    // O(1) tá»‘t nháº¥t; O(n) trung bĂ¬nh vĂ  xáº¥u nháº¥t.
    if (!node) return 0;
    if (node->member.id == id) return node;
    Node* found = FindByDfs(node->firstChild, id);
    return found ? found : FindByDfs(node->nextSibling, id);
}

void PrintDfs(Node* node, std::ostream& output, int depth) {
    // DFS in cĂ¢y: O(n) thá»i gian, O(h) ngÄƒn xáº¿p Ä‘á»‡ quy.
    if (!node) return;
    for (int i = 0; i < depth; ++i) output << "  ";
    output << "|- " << node->member.name << " [ID: " << node->member.id
           << "] " << node->member.birthDate << '\n';
    PrintDfs(node->firstChild, output, depth + 1);
    PrintDfs(node->nextSibling, output, depth);
}

void PrintBfsByGeneration(Node* root, std::ostream& output) {
    // BFS: O(n) thá»i gian, O(w) bá»™ nhá»› vá»›i w lĂ  bá» rá»™ng lá»›n nháº¥t cá»§a cĂ¢y.
    Queue queue;
    for (Node* current = root; current; current = current->nextSibling) queue.Push(current, 1);
    int generation = 0;
    while (queue.front) {
        QueueNode* item = queue.Pop();
        if (item->depth != generation) { generation = item->depth; output << "\nThe he " << generation << ":\n"; }
        output << "  " << item->treeNode->member.name << " (ID: " << item->treeNode->member.id
               << ", " << item->treeNode->member.birthDate << ")\n";
        for (Node* child = item->treeNode->firstChild; child; child = child->nextSibling)
            queue.Push(child, item->depth + 1);
        delete item;
    }
}

namespace {
int GetDepth(Node* node) {
    int depth = 0;
    while (node) { ++depth; node = node->parent; }
    return depth;
}
}

namespace {
Node* FindLcaFromNodes(Node* first, Node* second) {
    int firstDepth = GetDepth(first);
    int secondDepth = GetDepth(second);
    while (firstDepth > secondDepth) { first = first->parent; --firstDepth; }
    while (secondDepth > firstDepth) { second = second->parent; --secondDepth; }
    while (first && second && first != second) {
        first = first->parent;
        second = second->parent;
    }
    return first == second ? first : 0;
}
}

Node* FindLowestCommonAncestor(Node* root, int firstId, int secondId) {
    // Hai láº§n tĂ¬m O(n), sau Ä‘Ă³ leo theo parent O(h); bá»™ nhá»› phá»¥ O(1).
    return FindLcaFromNodes(FindByDfs(root, firstId), FindByDfs(root, secondId));
}

int GetRelationshipDistance(Node* root, int firstId, int secondId) {
    // Hai láº§n tĂ¬m O(n), leo cĂ¢y O(h), bá»™ nhá»› phá»¥ O(1); trĂ¡nh tĂ¬m cĂ¢y thĂªm láº§n ná»¯a.
    Node* first = FindByDfs(root, firstId);
    Node* second = FindByDfs(root, secondId);
    if (!first || !second) return -1;
    Node* ancestor = FindLcaFromNodes(first, second);
    if (!ancestor) return -1;
    return GetDepth(first) + GetDepth(second) - 2 * GetDepth(ancestor);
}
namespace {
void AppendDetached(Node*& head, Node*& tail, Node* node) {
    node->nextSibling = 0;
    if (!head) head = node;
    else tail->nextSibling = node;
    tail = node;
}

void JoinChains(Node*& head, Node*& tail, Node* chainHead, Node* chainTail) {
    if (!chainHead) return;
    if (!head) head = chainHead;
    else tail->nextSibling = chainHead;
    tail = chainTail;
}

Node* QuickSortSiblingChain(Node* head, Node*& tail) {
    if (!head || !head->nextSibling) {
        tail = head;
        return head;
    }

    // Chá»n ngĂ y sinh nĂºt Ä‘áº§u lĂ m pivot; phĂ¢n vĂ¹ng báº±ng cĂ¡ch thĂ¡o/gáº¯n láº¡i liĂªn káº¿t.
    Node* slow = head;
    Node* fast = head;
    while (fast->nextSibling && fast->nextSibling->nextSibling) {
        slow = slow->nextSibling;
        fast = fast->nextSibling->nextSibling;
    }
    const std::string pivotDate = slow->member.birthDate;
    Node* newerHead = 0; Node* newerTail = 0;
    Node* equalHead = 0; Node* equalTail = 0;
    Node* olderHead = 0; Node* olderTail = 0;

    while (head) {
        Node* current = head;
        head = head->nextSibling;
        if (CompareDates(current->member.birthDate, pivotDate) > 0)
            AppendDetached(newerHead, newerTail, current);
        else if (CompareDates(current->member.birthDate, pivotDate) < 0)
            AppendDetached(olderHead, olderTail, current);
        else
            AppendDetached(equalHead, equalTail, current);
    }

    if (newerHead) newerHead = QuickSortSiblingChain(newerHead, newerTail);
    if (olderHead) olderHead = QuickSortSiblingChain(olderHead, olderTail);

    Node* sortedHead = 0; Node* sortedTail = 0;
    JoinChains(sortedHead, sortedTail, newerHead, newerTail);
    JoinChains(sortedHead, sortedTail, equalHead, equalTail);
    JoinChains(sortedHead, sortedTail, olderHead, olderTail);
    tail = sortedTail;
    return sortedHead;
}
}

void SortSiblingsByBirthDate(Node*& firstSibling) {
    // Quick Sort relink node: O(n log n) trung bĂ¬nh, O(n^2) xáº¥u nháº¥t,
    // O(log n) ngÄƒn xáº¿p trung bĂ¬nh; khĂ´ng sao chĂ©p/Ä‘á»•i dá»¯ liá»‡u Member.
    Node* tail = 0;
    firstSibling = QuickSortSiblingChain(firstSibling, tail);
}
void CollectStatistics(Node* root, FamilyStats& stats) {
    // ThÄƒm má»—i nĂºt má»™t láº§n: O(n) thá»i gian, O(h) ngÄƒn xáº¿p.
    stats = FamilyStats(); CountRecursive(root, 1, stats);
}









namespace {
std::string LowerAscii(const std::string& value) {
    std::string result = value;
    for (std::size_t i = 0; i < result.size(); ++i)
        if (result[i] >= 'A' && result[i] <= 'Z') result[i] = static_cast<char>(result[i] - 'A' + 'a');
    return result;
}
void SearchNameRecursive(Node* node, const std::string& query, std::ostream& output, int& count) {
    if (!node) return;
    if (LowerAscii(node->member.name).find(query) != std::string::npos) {
        output << "  ID " << node->member.id << " | " << node->member.name << " | " << node->member.birthDate << '\n';
        ++count;
    }
    SearchNameRecursive(node->firstChild, query, output, count);
    SearchNameRecursive(node->nextSibling, query, output, count);
}
int ExtractYear(const std::string& date) {
    if (!IsValidDate(date)) return -1;
    return (date[6]-'0')*1000 + (date[7]-'0')*100 + (date[8]-'0')*10 + date[9]-'0';
}
void CountYearRecursive(Node* node, int year, int& births, int& deaths) {
    if (!node) return;
    if (ExtractYear(node->member.birthDate) == year) ++births;
    if (ExtractYear(node->member.deathDate) == year) ++deaths;
    CountYearRecursive(node->firstChild, year, births, deaths);
    CountYearRecursive(node->nextSibling, year, births, deaths);
}
}

void SearchByNameContains(Node* root, const std::string& query, std::ostream& output, int& count) {
    // Duyá»‡t tuyáº¿n tĂ­nh toĂ n cĂ¢y: best O(1) náº¿u khá»›p á»Ÿ nĂºt Ä‘áº§u; average/worst O(n), stack O(h).
    count = 0;
    if (query.empty()) return;
    SearchNameRecursive(root, LowerAscii(query), output, count);
}

void CountYear(Node* root, int year, int& births, int& deaths) {
    // Thá»‘ng kĂª pháº£i xĂ©t má»i thĂ nh viĂªn: O(n) thá»i gian, O(h) ngÄƒn xáº¿p Ä‘á»‡ quy.
    births = 0; deaths = 0; CountYearRecursive(root, year, births, deaths);
}