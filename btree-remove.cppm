export module btree:remove;
import :db;
import :retrieve;

export namespace btree {
template <typename Tp> db::nnid find_bro_in_node(db::nnid parent, db::nnid p) {
  auto &node = db::current()->read<Tp>(parent);
  if (p == node.p0)
    return node.p0;

  for (auto i = 0U; i < node.size; i++) {
    if (node.k[i].pi == p)
      return node.k[i].pi;
  }
  // unreachable
  throw db::inconsistency_error{};
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

template <typename Tp> bool remove(db::nnid *root, db::nnid y) {
  db::nnid p{};
  if (!retrieve<Tp>(*root, y, &p))
    return false;

  auto &node = db::current()->read<Tp>(p);
  if (node.leaf) {
    auto i = find_y_in_node<Tp>(p, y);
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

    auto p1 = find_bro_in_node<Tp>(node.parent, p);
    auto &p1node = db::current()->read<Tp>(p1);
    if (1 + nsz + p1node.size <= db::node_limit) {
      // merge
      throw 0;
    }
  }

  return false;
}
} // namespace btree
