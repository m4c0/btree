export module btree:insert;
import :db;

export namespace btree {
template <typename Tp>
void insert_entry_in_p(db::nnid s, const db::node<Tp> &node, db::key<Tp> k) {
  for (auto i = 0; i < node.size; i++) {
    if (k.xi < node.k[i].xi) {
      db::current()->insert_entry(s, i, k);
      return;
    }
  }
  db::current()->insert_entry(s, node.size, k);
}

template <typename Tp> bool insert(db::nnid *r, db::nnid y, Tp v) {
  db::nnid s{};
  if (retrieve<Tp>(*r, y, &s))
    return false;

  if (!s) {
    *r = db::current()->create_node({}, true);
    db::current()->insert_entry(*r, 0, db::key<Tp>{y, v});
    return true;
  }

  auto node = db::current()->read<Tp>(s);
  if (node.size == db::node_limit) {
    // split
    return true;
  }

  insert_entry_in_p(s, node, db::key<Tp>{y, v});
  return true;
}
} // namespace btree
