export module poc;
import btree;
import silog;

class test_failed {};

void check(auto &t, unsigned id) {
  if (!t.has(btree::db::nnid{id})) {
    silog::log(silog::debug, "missing id %d", id);
    throw test_failed{};
  }
}
void insert(auto &t, unsigned id) { t.insert(btree::db::nnid{id}, id * 100); }

void run() {
  using id = btree::db::nnid;

  btree::db::storage s{0L};
  btree::db::current() = &s;

  btree::tree<long> t{};

  insert(t, 6);
  insert(t, 1);
  insert(t, 7);
  insert(t, 5);
  insert(t, 2);

  check(t, 1);
  check(t, 2);
  check(t, 5);
  check(t, 6);
  check(t, 7);
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
