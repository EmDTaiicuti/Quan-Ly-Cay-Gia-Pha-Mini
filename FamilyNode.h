#ifndef FAMILY_NODE_H
#define FAMILY_NODE_H

#include <string>
// Thông tin của một người trong gia phả; các ID dùng để định danh và liên kết.
struct Member {
    int id;                    // ID duy nhất của thành viên
    int spouseId;              // ID vợ/chồng; bằng 0 nếu chưa khai báo
    std::string name;          // họ tên đầy đủ
    char gender;               // giới tính: M-ale, F-emale , 0-biết
    std::string birthDate;     // ngày sinh theo định dạng dd-mm-yyyy
    std::string notes;         // ghi chú tự do về thành viên.

    // giá trị mặc định giúp một Member mới luôn ở trạng thái hợp lệ ban đầu.
    Member() : id(0), spouseId(0), name(), gender('O'), birthDate(), notes() {}
};


// một nút trong cây con trưởng - anh em ruột.
struct Node {
    Member member;             // dữ liệu người mà nút này đại diện.
    Node* parent;              // nút cha; null nếu đây là một nút gốc.
    Node* firstChild;          // con đầu tiên trong danh sách con.
    Node* lastChild;           // con cuối cùng, giúp thêm con mới trong O(1).
    Node* nextSibling;         // anh/chị/em kế tiếp cùng cha.
 // parentNode được truyền khi tạo nút con; các liên kết con/anh em ban đầu rỗng.
    explicit Node(const Member& value, Node* parentNode = 0)
        : member(value), parent(parentNode), firstChild(0), lastChild(0), nextSibling(0) {}
};
// Trạng thái tổng thể của cây gia phả và các thông tin đuôi/số lượng để quản lý nhanh.
struct FamilyTree {
    Node* root;                // nút gốc đầu tiên; các gốc khác nối qua nextSibling.
    Node* lastRoot;            // nút gốc cuối cùng, giúp thêm gốc trong O(1).
    int memberCount;           // tổng số nút hiện có trong cây.

    FamilyTree() : root(0), lastRoot(0), memberCount(0) {}
};
