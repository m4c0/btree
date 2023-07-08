export module poc;
import btree;

class tree {
public:
};

extern "C" int main() {
  btree::db::storage s{0L};
  btree::db::current() = &s;
  auto _ = s.read<long>(btree::db::nnid{99});

  tree t{};
}
