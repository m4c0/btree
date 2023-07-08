export module poc;
import btree;

extern "C" int main() {
  btree::db::storage s{0L};
  btree::db::current() = &s;

  btree::tree<long> t{};
  return t.has(btree::db::nnid{99}) ? 1 : 0;
}
