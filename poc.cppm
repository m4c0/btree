export module poc;
import btree;
import rng;
import silog;

using namespace btree;

class test_failed {};

void dump_node(db::nnid id, unsigned ind, unsigned &la) {
  auto &node = db::current()->read<long>(id);
  silog::log(silog::debug, "%*snode: %d - s:%d p:%d l:%d", ind, "", id.index(),
             node.size, node.parent.index(), ind);

  ind++;
  if (!node.leaf)
    dump_node(node.p0, ind, la);
  for (auto i = 0; i < node.size; i++) {
    auto k = node.k[i];
    char fill = (k.ai <= la) ? '!' : ' ';
    silog::log(silog::debug, "%c%*si=%d k=%d v=%ld", fill, ind - 1, "", i,
               k.xi.index(), k.ai);
    la = k.ai;
    if (!node.leaf)
      dump_node(k.pi, ind, la);
  }
}
void dump_tree(db::nnid id) {
  unsigned la{};
  dump_node(id, 0, la);
}

void check_all(db::nnid id, db::nnid parent, unsigned &la) {
  auto &node = db::current()->read<long>(id);
  if (node.parent && node.size < db::node_lower_limit) {
    silog::log(silog::error, "node %d with size %d", id.index(), node.size);
    throw 0;
  }
  if (node.parent != parent) {
    silog::log(silog::error, "node %d with invalid parent %d exp=%d",
               id.index(), node.parent.index(), parent.index());
    throw 0;
  }

  if (!node.leaf)
    check_all(node.p0, id, la);

  for (auto i = 0; i < node.size; i++) {
    auto k = node.k[i];
    if (k.ai <= la)
      throw 0;

    la = k.ai;
    if (!node.leaf)
      check_all(k.pi, id, la);
  }
}
void check_all(db::nnid root) {
  unsigned la{};
  try {
    check_all(root, {}, la);
  } catch (...) {
    silog::log(silog::error, "inconsistent tree with root = %d", root.index());
    throw test_failed{};
  };
}

void check(auto &t, unsigned id) {
  if (!t.has(db::nnid{id})) {
    silog::log(silog::error, "missing id %d", id);
    throw test_failed{};
  }
  check_all(t.root());
}
void insert(auto &t, unsigned id) {
  if (!t.insert(db::nnid{id}, id * 100)) {
    silog::log(silog::error, "insert failed for id %d", id);
    throw test_failed{};
  }
}
void remove(auto &t, unsigned id) {
  silog::log(silog::debug, "remove %d", id);
  if (!t.remove(db::nnid{id})) {
    silog::log(silog::error, "remove failed for id %d", id);
    throw test_failed{};
  }
  if (t.has(db::nnid{id})) {
    silog::log(silog::error, "element id %d still there", id);
    throw test_failed{};
  }
}

void run(auto &t) {
  using id = db::nnid;

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

  silog::log(silog::info, "inserting");
  for (auto n : all) {
    insert(t, n);
    check(t, n);
  }
  silog::log(silog::info, "checking all again");
  for (auto n : all) {
    check(t, n);
  }
  silog::log(silog::info, "cleaning up");
  for (auto i = 0; i < max; i++) {
    remove(t, all[i]);
    for (auto j = i + 1; j < max; j++) {
      check(t, all[j]);
    }
  }
}
extern "C" int main() {
  db::storage s{0L};
  db::current() = &s;

  tree<long> t{};
  try {
    silog::log(silog::warning, "test started");
    run(t);
    silog::log(silog::warning, "test passed");
    return 0;
  } catch (test_failed) {
    silog::log(silog::error, "test failed");
  } catch (db::inconsistency_error) {
    silog::log(silog::error, "db error");
  } catch (...) {
    silog::log(silog::error, "something broke");
  }
  dump_tree(t.root());
  return 1;
}
