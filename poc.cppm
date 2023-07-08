export module poc;
import btree;

extern "C" int main() {
  using id = btree::db::nnid;

  btree::db::storage s{0L};
  btree::db::current() = &s;

  btree::tree<long> t{};

  for (auto i = 0U; i < 10000; i++) {
    if (!t.insert(id{i}, i))
      return 1;
  }

  return t.has(id{99}) ? 0 : 1;
}
