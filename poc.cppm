export module poc;
import btree;
import silog;

int run() {
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
extern "C" int main() {
  try {
    return run();
  } catch (btree::db::inconsistency_error) {
    silog::log(silog::error, "db error");
    return 1;
  }
}
