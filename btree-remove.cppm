export module btree:remove;
import :db;
export import :log;
import :retrieve;
export import hai;

export namespace btree {
template <typename Tp> unsigned find_bro_in_node(db::nnid parent, db::nnid *p) {
  auto &node = db::current()->read<Tp>(parent);
  if (*p == node.p0)
    return 0;

  for (auto i = 0U; i < node.size - 1; i++) {
    if (node.k[i].pi == *p)
      return i + 1;
  }

  *p = node.k[node.size - 2].pi;
  return node.size - 1;
}

template <typename Tp> unsigned find_y_in_node(db::nnid n, db::nnid y) {
  auto &node = db::current()->read<Tp>(n);
  for (auto i = 0U; i < node.size; i++) {
    if (node.k[i].xi == y)
      return i;
  }
  // unreachable
  throw db::inconsistency_error{};
}

template <typename Tp> auto find_first_leaf(db::nnid n) {
  auto node = db::current()->read<Tp>(n);
  if (node.leaf)
    return n;
  return find_first_leaf<Tp>(node.k[0].pi);
}

template <typename Tp> bool catenate(db::nnid p) {
  auto q = db::current()->read<Tp>(p).parent;

  auto idx = find_bro_in_node<Tp>(q, &p);
  auto [yj, aj, p1] = db::current()->read<Tp>(q).k[idx];
  auto &node = db::current()->read<Tp>(p);
  auto &p1node = db::current()->read<Tp>(p1);
  auto fsz = 1 + node.size + p1node.size;
  db::key<Tp> kj{yj, aj, p1node.p0};
  if (fsz <= db::node_limit) {
    log("catenate %d %d", p.index(), p1.index());
    db::current()->append_entry(p, kj);
    for (auto i = 0; i < p1node.size; i++) {
      db::current()->append_entry(p, p1node.k[i]);
    }

    db::current()->remove_entry<Tp>(q, idx);
    db::current()->delete_node(p1);
    return true;
  }

  log("underflow %d %d -- %d", p.index(), p1.index(), fsz);

  hai::array<db::key<Tp>> fk{fsz};
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
  }
  fk[mid].pi = p1;
  db::current()->set_entry(q, idx, fk[mid]);
  for (auto i = mid + 1; i < fsz; i++) {
    db::current()->append_entry(p1, fk[i]);
  }
  return true;
}

template <typename Tp> bool remove(db::nnid *root, db::nnid y) {
  db::nnid p{};
  if (!retrieve<Tp>(*root, y, &p))
    return false;

  auto &node = db::current()->read<Tp>(p);
  auto i = find_y_in_node<Tp>(p, y);
  if (node.leaf) {
    auto nsz = db::current()->remove_entry<Tp>(p, i);
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
    auto ll = find_first_leaf<Tp>(node.k[i].pi);
    auto k0ll = db::current()->read<Tp>(ll).k[0];
    k0ll.pi = node.k[i].pi;
    db::current()->set_entry(p, i, k0ll);

    auto nsz = db::current()->remove_entry<Tp>(ll, 0);
    if (nsz >= db::node_lower_limit)
      return true;

    return false;
  }

  return catenate<Tp>(p);
}
} // namespace btree
