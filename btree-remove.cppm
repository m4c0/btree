export module btree:remove;
import :db;
import :retrieve;

export namespace btree {
template <typename Tp> bool remove(db::nnid *root, db::nnid y) {
  db::nnid n{};
  if (!retrieve<Tp>(*root, y, &n))
    return false;

  auto &node = db::current()->read<Tp>(n);
  if (node.leaf) {
    for (auto i = 0U; i < node.size; i++) {
      if (node.k[i].xi != y)
        continue;

      auto nsz = db::current()->remove_entry<Tp>(n, i);
      if (nsz > db::node_lower_limit)
        return true;

      return false;
    }
  }

  return false;
}
} // namespace btree
