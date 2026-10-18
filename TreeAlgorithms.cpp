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