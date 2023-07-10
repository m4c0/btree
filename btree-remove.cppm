export module btree:remove;
import :db;
import :retrieve;
export import hai;

namespace btree {
unsigned find_bro_in_node(db::nnid parent, db::nnid *p) {
  auto &node = db::current()->read(parent);
  if (*p == node.p0)
    return 0;

  for (auto i = 0U; i < node.size - 1; i++) {
    if (node.k[i].pi == *p)
      return i + 1;
  }

  if (node.size == 1) {
    *p = node.p0;
    return 0;
  }

  *p = node.k[node.size - 2].pi;
  return node.size - 1;
}

unsigned find_y_in_node(db::nnid n, db::nnid y) {
  auto &node = db::current()->read(n);
  for (auto i = 0U; i < node.size; i++) {
    if (node.k[i].xi == y)
      return i;
  }
  // unreachable
  throw db::inconsistency_error{};
}

auto find_first_leaf(db::nnid n) {
  auto node = db::current()->read(n);
  if (node.leaf)
    return n;
  return find_first_leaf(node.p0);
}

void change_parenthood(db::nnid from, db::nnid to) {
  auto &node = db::current()->read(from);
  if (node.leaf)
    return;

  db::current()->set_parent(node.p0, to);
  for (auto i = 0; i < node.size; i++) {
    db::current()->set_parent(node.k[i].pi, to);
  }
}

void catenate(db::nnid p) {
  auto q = db::current()->read(p).parent;
  if (!q)
    return;

  auto idx = find_bro_in_node(q, &p);
  auto [yj, aj, p1] = db::current()->read(q).k[idx];
  auto &node = db::current()->read(p);
  auto &p1node = db::current()->read(p1);
  auto fsz = 1 + node.size + p1node.size;
  db::key kj{yj, aj, p1node.p0};
  if (fsz <= db::node_limit) {
    db::current()->append_entry(p, kj);
    for (auto i = 0; i < p1node.size; i++) {
      db::current()->append_entry(p, p1node.k[i]);
    }

    change_parenthood(p1, p);
    db::current()->delete_node(p1);

    if (db::current()->remove_entry(q, idx) < db::node_lower_limit)
      catenate(q);

    return;
  }

  // underflow

  hai::array<db::key> fk{fsz};
  for (auto i = 0; i < node.size; i++) {
    fk[i] = node.k[i];
  }
  fk[node.size] = kj;
  for (auto i = 0; i < p1node.size; i++) {
    fk[i + node.size + 1] = p1node.k[i];
  }

  db::current()->set_size(p, 0);
  db::current()->set_size(p1, 0);

  auto mid = fsz / 2;
  for (auto i = 0; i < mid; i++) {
    db::current()->append_entry(p, fk[i]);
    if (!node.leaf)
      db::current()->set_parent(fk[i].pi, p);
  }
  if (!node.leaf) {
    db::current()->set_p0(p1, fk[mid].pi);
    db::current()->set_parent(fk[mid].pi, p1);
  }
  fk[mid].pi = p1;
  db::current()->set_entry(q, idx, fk[mid]);
  for (auto i = mid + 1; i < fsz; i++) {
    db::current()->append_entry(p1, fk[i]);
    if (!node.leaf)
      db::current()->set_parent(fk[i].pi, p1);
  }
}

bool remove(db::nnid *root, db::nnid y) {
  db::nnid p{};
  if (!retrieve(*root, y, &p))
    return false;

  auto &node = db::current()->read(p);
  auto i = find_y_in_node(p, y);
  if (node.leaf) {
    auto nsz = db::current()->remove_entry(p, i);
    if (nsz >= db::node_lower_limit)
      return true;

    if (!node.parent) {
      if (nsz == 0) {
        db::current()->delete_node(*root);
        *root = {};
      }
      return true;
    }
  } else {
    auto ll = find_first_leaf(node.k[i].pi);
    auto k0ll = db::current()->read(ll).k[0];
    k0ll.pi = node.k[i].pi;
    db::current()->set_entry(p, i, k0ll);

    auto nsz = db::current()->remove_entry(ll, 0);
    if (nsz >= db::node_lower_limit)
      return true;

    p = ll;
  }

  catenate(p);

  auto &rn = db::current()->read(*root);
  if (rn.size == 0) {
    auto new_root = rn.p0;
    db::current()->delete_node(*root);
    db::current()->set_parent(new_root, {});
    *root = new_root;
  }
  return true;
}
} // namespace btree
