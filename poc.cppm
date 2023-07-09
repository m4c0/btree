export module poc;
import btree;
import rng;
import silog;

using namespace btree;

class test_failed {};

void dump_node(db::nnid id, unsigned ind) {
  auto &node = db::current()->read<long>(id);
  log("%*snode: %d", ind, "", id.index());

  ind++;
  if (!node.leaf)
    dump_node(node.p0, ind);
  for (auto i = 0; i < node.size; i++) {
    auto k = node.k[i];
    log("%*sk=%d v=%ld", ind, "", k.xi.index(), k.ai);
    if (!node.leaf)
      dump_node(k.pi, ind);
  }
}

void check(auto &t, unsigned id) {
  if (!t.has(db::nnid{id})) {
    log("missing id %d", id);
    dump_node(t.root(), 0);
    throw test_failed{};
  }
}
void insert(auto &t, unsigned id) { t.insert(db::nnid{id}, id * 100); }

void run() {
  using id = db::nnid;

  db::storage s{0L};
  db::current() = &s;

  constexpr const auto max = 10240;
  unsigned all[max];
  for (auto i = 0U; i < max; i++) {
    all[i] = i;
  }
  for (auto i = 0U; i < max; i++) {
    for (auto j = 0U; j < max; j++) {
      if (rng::randf() > 0.5f)
        continue;
      auto tmp = all[j];
      all[j] = all[i];
      all[i] = tmp;
    }
  }

  tree<long> t{};
  silog::log(silog::info, "inserting");
  for (auto n : all) {
    log("insert/check %d", n);
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
  } catch (db::inconsistency_error) {
    silog::log(silog::error, "db error");
  } catch (...) {
    silog::log(silog::error, "something broke");
  }
  return 1;
}
