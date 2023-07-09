export module poc;
import btree;
import rng;
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

  constexpr const auto max = 10240;
  unsigned all[max];
  for (auto i = 0U; i < max; i++) {
    all[i] = i;
  }
  for (auto i = 0U; i < max; i++) {
    for (auto j = i + 1; j < max; j++) {
      if (rng::randf() > 0.5f)
        continue;
      auto tmp = all[j];
      all[j] = all[i];
      all[i] = tmp;
    }
  }

  btree::tree<long> t{};
  silog::log(silog::info, "inserting");
  for (auto n : all) {
    silog::log(silog::debug, "insert/check %d", n);
    insert(t, n);
    check(t, n);
  }
  silog::log(silog::info, "checking");
  for (auto n : all) {
    check(t, n);
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
