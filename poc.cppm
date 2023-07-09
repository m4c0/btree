export module poc;
import btree;
import rng;
import silog;

using namespace btree;

class test_failed {};

void dump_node(db::nnid id, unsigned ind, unsigned &la) {
  auto &node = db::current()->read<long>(id);
  log("%*snode: %d - s:%d p:%d l:%d", ind, "", id.index(), node.size,
      node.parent.index(), ind);

  ind++;
  if (!node.leaf)
    dump_node(node.p0, ind, la);
  for (auto i = 0; i < node.size; i++) {
    auto k = node.k[i];
    char fill = (k.ai <= la) ? '!' : ' ';
    log("%c%*si=%d k=%d v=%ld", fill, ind - 1, "", i, k.xi.index(), k.ai);
    la = k.ai;
    if (!node.leaf)
      dump_node(k.pi, ind, la);
  }
}
void dump_tree(db::nnid id) {
  unsigned la{};
  dump_node(id, 0, la);
}

void check_all(db::nnid id, unsigned &la) {
  auto &node = db::current()->read<long>(id);
  if (!node.leaf)
    check_all(node.p0, la);

  for (auto i = 0; i < node.size; i++) {
    auto k = node.k[i];
    if (k.ai <= la)
      throw 0;

    la = k.ai;
    if (!node.leaf)
      check_all(k.pi, la);
  }
}
void check_all(db::nnid root) {
  unsigned la{};
  try {
    check_all(root, la);
  } catch (...) {
    log("inconsistent tree with root = %d", root.index());
    dump_tree(root);
    throw test_failed{};
  };
}

void check(auto &t, unsigned id) {
  if (!t.has(db::nnid{id})) {
    log("missing id %d", id);
    dump_tree(t.root());
    throw test_failed{};
  }
  check_all(t.root());
}
void insert(auto &t, unsigned id) {
  if (!t.insert(db::nnid{id}, id * 100)) {
    log("insert failed for id %d", id);
    dump_tree(t.root());
    throw test_failed{};
  }
}

void run() {
  using id = db::nnid;

  db::storage s{0L};
  db::current() = &s;

  constexpr const auto max = 10240;
  unsigned all[max];
  for (auto i = 0U; i < max; i++) {
    all[i] = i + 1;
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
