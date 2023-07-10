export module btree:remove;
import :db;
import :retrieve;
export import hai;

namespace btree {
unsigned find_bro_in_node(db::storage *dbs, db::nnid parent, db::nnid *p) {
  auto &node = dbs->read(parent);
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

unsigned find_y_in_node(db::storage *dbs, db::nnid n, db::nnid y) {
  auto &node = dbs->read(n);
  for (auto i = 0U; i < node.size; i++) {
    if (node.k[i].xi == y)
      return i;
  }
  // unreachable
  throw db::inconsistency_error{};
}

auto find_first_leaf(db::storage *dbs, db::nnid n) {
  auto node = dbs->read(n);
  if (node.leaf)
    return n;
  return find_first_leaf(dbs, node.p0);
}

void change_parenthood(db::storage *dbs, db::nnid from, db::nnid to) {
  auto &node = dbs->read(from);
  if (node.leaf)
    return;

  dbs->set_parent(node.p0, to);
  for (auto i = 0; i < node.size; i++) {
    dbs->set_parent(node.k[i].pi, to);
  }
}

void catenate(db::storage *dbs, db::nnid p) {
  auto q = dbs->read(p).parent;
  if (!q)
    return;

  auto idx = find_bro_in_node(dbs, q, &p);
  auto [yj, aj, p1] = dbs->read(q).k[idx];
  auto &node = dbs->read(p);
  auto &p1node = dbs->read(p1);
  auto fsz = 1 + node.size + p1node.size;
  db::key kj{yj, aj, p1node.p0};
  if (fsz <= db::node_limit) {
    dbs->append_entry(p, kj);
    for (auto i = 0; i < p1node.size; i++) {
      dbs->append_entry(p, p1node.k[i]);
    }

    change_parenthood(dbs, p1, p);
    dbs->delete_node(p1);

    if (dbs->remove_entry(q, idx) < db::node_lower_limit)
      catenate(dbs, q);

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

  dbs->set_size(p, 0);
  dbs->set_size(p1, 0);

  auto mid = fsz / 2;
  for (auto i = 0; i < mid; i++) {
    dbs->append_entry(p, fk[i]);
    if (!node.leaf)
      dbs->set_parent(fk[i].pi, p);
  }
  if (!node.leaf) {
    dbs->set_p0(p1, fk[mid].pi);
    dbs->set_parent(fk[mid].pi, p1);
  }
  fk[mid].pi = p1;
  dbs->set_entry(q, idx, fk[mid]);
  for (auto i = mid + 1; i < fsz; i++) {
    dbs->append_entry(p1, fk[i]);
    if (!node.leaf)
      dbs->set_parent(fk[i].pi, p1);
  }
}

bool remove(db::storage *dbs, db::nnid *root, db::nnid y) {
  db::nnid p{};
  if (!retrieve(dbs, *root, y, &p))
    return false;

  auto &node = dbs->read(p);
  auto i = find_y_in_node(dbs, p, y);
  if (node.leaf) {
    auto nsz = dbs->remove_entry(p, i);
    if (nsz >= db::node_lower_limit)
      return true;

    if (!node.parent) {
      if (nsz == 0) {
        dbs->delete_node(*root);
        *root = {};
      }
      return true;
    }
  } else {
    auto ll = find_first_leaf(dbs, node.k[i].pi);
    auto k0ll = dbs->read(ll).k[0];
    k0ll.pi = node.k[i].pi;
    dbs->set_entry(p, i, k0ll);

    auto nsz = dbs->remove_entry(ll, 0);
    if (nsz >= db::node_lower_limit)
      return true;

    p = ll;
  }

  catenate(dbs, p);

  auto &rn = dbs->read(*root);
  if (rn.size == 0) {
    auto new_root = rn.p0;
    dbs->delete_node(*root);
    dbs->set_parent(new_root, {});
    *root = new_root;
  }
  return true;
}
} // namespace btree
