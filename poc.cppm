export module poc;
import btree;
import silog;

class test_failed {};

void run() {
  using id = btree::db::nnid;

  btree::db::storage s{0L};
  btree::db::current() = &s;

  btree::tree<long> t{};

  t.insert(id{4}, 400);
  t.insert(id{1}, 100);
  t.insert(id{3}, 300);
  t.insert(id{2}, 200);
  for (auto i = 1U; i < 4; i++) {
    if (!t.has(id{i})) {
      silog::log(silog::debug, "missing id %d", i);
      throw test_failed{};
    }
  }
}
extern "C" int main() {
  try {
    silog::log(silog::error, "test started");
    run();
    silog::log(silog::error, "test passed");
    return 0;
  } catch (test_failed) {
    silog::log(silog::error, "test failed");
  } catch (btree::db::inconsistency_error) {
    silog::log(silog::error, "db error");
  } catch (...) {
    silog::log(silog::error, "something broke");
  }
  return 1;
}
