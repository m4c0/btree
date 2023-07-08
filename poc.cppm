export module poc;
import btree;

class tree {
public:
};

extern "C" int main() {
  btree::db::storage s{0L};
  btree::db::current() = &s;

  tree t{};
}
