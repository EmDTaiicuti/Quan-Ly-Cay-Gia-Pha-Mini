#include <algorithm>
#include <cctype>

// ===================== HÀNG ĐỢI TỰ CÀI ĐẶT =====================
// (Không dùng std::queue)

struct QueueNode {
    FamilyNode* treeNode;
    QueueNode* next;
};

struct NodeQueue {
    QueueNode* head;   // Đầu hàng: nơi lấy ra
    QueueNode* tail;   // Cuối hàng: nơi thêm vào
    int size;
};

static void QueueInit(NodeQueue& queue) {
    queue.head = nullptr;
    queue.tail = nullptr;
    queue.size = 0;
}

static void Enqueue(NodeQueue& queue, FamilyNode* treeNode) {
    QueueNode* item = new QueueNode{treeNode, nullptr};

    if (queue.tail == nullptr) {
        // Hàng rỗng: head và tail cùng trỏ vào phần tử đầu tiên
        queue.head = item;
        queue.tail = item;
    } else {
        // Nối vào sau tail rồi dời tail
        queue.tail->next = item;
        queue.tail = item;
    }

    ++queue.size;
}

static FamilyNode* Dequeue(NodeQueue& queue) {
    QueueNode* item = queue.head;
    FamilyNode* treeNode = item->treeNode;

    queue.head = item->next;

    if (queue.head == nullptr) {
        // Vừa lấy phần tử cuối -> hàng rỗng
        queue.tail = nullptr;
    }

    delete item;       // Giải phóng ô hàng đợi
    --queue.size;

    return treeNode;
}

static std::string ToLower(const std::string& text) {
    std::string result = text;

    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(result[i]))
        );
    }

    return result;
}
// ===================== DFS =====================

// DFS tìm đường "xuất xứ": in từ người cần tìm ngược lên thủy tổ.
// Cách hoạt động: duyệt sâu xuống con (firstChild),
// duyệt ngang qua anh em (nextSibling).
// Khi tìm thấy mục tiêu thì trả true; khi đệ quy QUAY LUI,
// mỗi tổ tiên trên đường về sẽ tự in tên mình,
// nhờ đó đường đi được in ra mà không cần mảng/ngăn xếp phụ.
// Big-O: Thời gian O(n) (xấu nhất duyệt hết cây);
// Bộ nhớ O(h) cho ngăn xếp đệ quy (h = chiều cao).

bool DfsPrintLineage(FamilyNode* node, int targetId) {
    for (FamilyNode* current = node;
         current != nullptr;
         current = current->nextSibling) {

        if (current->data.id == targetId) {
            std::cout << DescribeMember(current->data);
            return true;
        }

        if (current->spouse != nullptr &&
            current->spouse->data.id == targetId) {

            // Mục tiêu là vợ/chồng nhập gia:
            // nối dòng họ thông qua người huyết thống.
            std::cout << DescribeMember(current->spouse->data)
                      << " [vợ/chồng của "
                      << current->data.name << "]";

            return true;
        }

        if (DfsPrintLineage(current->firstChild, targetId)) {
            // Tìm sâu xuống nhánh con
            std::cout << "  <-  "
                      << DescribeMember(current->data);

            // In khi quay lui
            return true;
        }
    }

    return false;
}


// DFS (tiền tự) tìm mọi thành viên có tên chứa từ khóa
// (không phân biệt hoa/thường với ký tự ASCII).
// Big-O: Thời gian O(n * L) với L là độ dài tên;
// Bộ nhớ O(h).

int DfsSearchByName(
    FamilyNode* node,
    const std::string& keyword
) {
    int found = 0;

    const std::string key = ToLower(keyword);

    for (FamilyNode* current = node;
         current != nullptr;
         current = current->nextSibling) {

        if (ToLower(current->data.name).find(key)
            != std::string::npos) {

            std::cout << "  - "
                      << DescribeMember(current->data)
                      << "  (Đời "
                      << GetDepth(current) + 1
                      << ")\n";

            ++found;
        }

        if (current->spouse != nullptr &&
            ToLower(current->spouse->data.name).find(key)
            != std::string::npos) {

            std::cout << "  - "
                      << DescribeMember(current->spouse->data)
                      << "  (vợ/chồng của "
                      << current->data.name
                      << ", Đời "
                      << GetDepth(current) + 1
                      << ")\n";

            ++found;
        }

        found += DfsSearchByName(
            current->firstChild,
            keyword
        );
    }

    return found;
}
// ===================== BFS =====================

// BFS theo từng thế hệ.
// Mỗi vòng while xử lý đúng "levelSize" phần tử
// = toàn bộ một đời.
// Con của họ (đi theo firstChild rồi nextSibling)
// được xếp hàng cho đời kế tiếp.
// Big-O: Thời gian O(n);
// Bộ nhớ O(w) với w là số người đông nhất trong một thế hệ.

void BfsPrintByGeneration(FamilyNode* root) {

    if (root == nullptr) {
        std::cout << "Cây gia phả đang rỗng.\n";
        return;
    }

    NodeQueue queue;

    QueueInit(queue);

    Enqueue(queue, root);

    int generation = 1;

    while (queue.size > 0) {

        int levelSize = queue.size;
        // Chốt số lượng của đời hiện tại
        // TRƯỚC khi thêm con vào hàng.

        std::cout << "Đời " << generation << ":\n";

        for (int i = 0; i < levelSize; ++i) {

            FamilyNode* current = Dequeue(queue);

            std::cout << "    "
                      << DescribeMember(current->data);

            if (current->spouse != nullptr) {
                std::cout << "  &  "
                          << DescribeMember(current->spouse->data);
            }

            std::cout << "\n";

            for (FamilyNode* child = current->firstChild;
                 child != nullptr;
                 child = child->nextSibling) {

                Enqueue(queue, child);
            }
        }

        ++generation;
    }
}
// ===================== ĐỘ SÂU, LCA, KHOẢNG CÁCH =====================

// Độ sâu = số cạnh đi ngược lên gốc nhờ con trỏ parent.
// Thủy tổ có độ sâu 0.
// Big-O: Thời gian O(h); Bộ nhớ O(1).

int GetDepth(const FamilyNode* node) {
    int depth = 0;

    while (node != nullptr && node->parent != nullptr) {
        node = node->parent;
        ++depth;
    }

    return depth;
}


// Tổ tiên chung gần nhất (LCA) bằng con trỏ parent:
// 1. Đưa người sâu hơn nhích lên cho cùng độ sâu với người kia.
// 2. Cả hai cùng nhích lên từng bước cho tới khi trùng nhau
//    -> đó là LCA.
// Yêu cầu a, b là người huyết thống
// (UI đã quy vợ/chồng về người huyết thống).
// Big-O: Thời gian O(h); Bộ nhớ O(1).

FamilyNode* FindLca(FamilyNode* a, FamilyNode* b) {
    if (a == nullptr || b == nullptr) {
        return nullptr;
    }

    int depthA = GetDepth(a);
    int depthB = GetDepth(b);

    while (depthA > depthB) {
        a = a->parent;
        --depthA;
    }

    while (depthB > depthA) {
        b = b->parent;
        --depthB;
    }

    while (a != b) {
        a = a->parent;
        b = b->parent;

        if (a == nullptr || b == nullptr) {
            return nullptr;
            // Khác cây (không xảy ra trong 1 cây hợp lệ).
        }
    }

    return a;
}
// Tính khoảng cách quan hệ giữa hai người.
//
// Công thức:
// distance = depthA + depthB - 2 * depthLca
//
// Trả về -1 nếu không có tổ tiên chung.
//
// Big-O:
// Thời gian O(h).
// Bộ nhớ O(1).

int FamilyDistance(
    FamilyNode* a,
    FamilyNode* b
) {
    FamilyNode* lca = FindLca(a, b);

    if (lca == nullptr) {
        return -1;
    }

    return GetDepth(a)
         + GetDepth(b)
         - 2 * GetDepth(lca);
}


// ===================== SẮP XẾP ANH EM =====================

// Selection Sort trên danh sách liên kết anh em.
//
// Sắp xếp năm sinh giảm dần:
// - Người sinh muộn nhất đứng đầu.
// - Nếu cùng năm thì so tiếp tháng và ngày.
// - Nếu ngày sinh bằng nhau thì giữ thứ tự cũ.
//
// Không hoán đổi dữ liệu của node.
// Chỉ thay đổi con trỏ nextSibling.
//
// Big-O:
// Thời gian O(k^2), với k là số anh em.
// Bộ nhớ O(1).

FamilyNode* SortSiblingList(FamilyNode* head) {
    FamilyNode* sortedHead = nullptr;
    FamilyNode* sortedTail = nullptr;

    while (head != nullptr) {
        FamilyNode* maxNode = head;
        FamilyNode* maxPrev = nullptr;
        FamilyNode* prev = head;

        // Tìm node có ngày sinh lớn nhất
        for (
            FamilyNode* current = head->nextSibling;
            current != nullptr;
            prev = current, current = current->nextSibling
        ) {
            if (
                DateToKey(current->data.birth)
                > DateToKey(maxNode->data.birth)
            ) {
                maxNode = current;
                maxPrev = prev;
            }
        }

        // Tháo maxNode khỏi danh sách còn lại
        if (maxPrev == nullptr) {
            // maxNode đang là đầu danh sách
            head = maxNode->nextSibling;
        } else {
            // Nối vòng qua maxNode
            maxPrev->nextSibling = maxNode->nextSibling;
        }

        maxNode->nextSibling = nullptr;

        // Nối maxNode vào đuôi danh sách đã sắp xếp
        if (sortedHead == nullptr) {
            sortedHead = maxNode;
            sortedTail = maxNode;
        } else {
            sortedTail->nextSibling = maxNode;
            sortedTail = maxNode;
        }
    }

    return sortedHead;
}


// Sắp xếp các con của một người.
// Cập nhật firstChild vì người con đứng đầu có thể thay đổi.
//
// Big-O: O(k^2).

void SortChildrenOf(FamilyNode* parent) {
    if (parent == nullptr) {
        return;
    }

    parent->firstChild = SortSiblingList(
        parent->firstChild
    );
}


// Sắp xếp đồng loạt mọi gia đình trong cây bằng đệ quy.
//
// Big-O:
// Tổng O(sum k_i^2), tối đa O(n^2).
// Bộ nhớ O(h).

void SortAllSiblings(FamilyNode* node) {
    if (node == nullptr) {
        return;
    }

    // Sắp xếp danh sách con của node hiện tại
    SortChildrenOf(node);

    // Tiếp tục sắp xếp các nhánh con
    for (
        FamilyNode* child = node->firstChild;
        child != nullptr;
        child = child->nextSibling
    ) {
        SortAllSiblings(child);
    }
}

// ===================== THỐNG KÊ =====================

static void CollectStats(const FamilyNode* node, TreeStats& stats) {
    ++stats.total;

    if (node->data.gender == 'M')
        ++stats.male;
    else
        ++stats.female;

    if (node->spouse != nullptr) {
        ++stats.total;

        if (node->spouse->data.gender == 'M')
            ++stats.male;
        else
            ++stats.female;
    }

    for (const FamilyNode* child = node->firstChild;
         child != nullptr;
         child = child->nextSibling) {
        CollectStats(child, stats);
    }
}

// Đếm số người trong nhánh gốc tại node (node + vợ/chồng + toàn bộ con cháu),
// KHÔNG tính anh em của node.
// Big-O: O(m) với m là kích thước nhánh.
int CountSubtree(const FamilyNode* node) {
    if (node == nullptr)
        return 0;

    int count = 1 + (node->spouse != nullptr ? 1 : 0);

    for (const FamilyNode* child = node->firstChild;
         child != nullptr;
         child = child->nextSibling) {
        count += CountSubtree(child);
    }

    return count;
}

// Chiều cao cây = số thế hệ (cây chỉ có thủy tổ -> 1).
// Big-O: Thời gian O(n); Bộ nhớ O(h).
int TreeHeight(const FamilyNode* node) {
    if (node == nullptr)
        return 0;

    int maxChildHeight = 0;

    for (const FamilyNode* child = node->firstChild;
         child != nullptr;
         child = child->nextSibling) {
        maxChildHeight = std::max(
            maxChildHeight,
            TreeHeight(child)
        );
    }

    return 1 + maxChildHeight;
}

// Big-O: O(n).
TreeStats ComputeStats(const FamilyNode* root) {
    TreeStats stats = {0, 0, 0, 0};

    if (root == nullptr)
        return stats;

    CollectStats(root, stats);

    stats.generations = TreeHeight(root);

    return stats;
}