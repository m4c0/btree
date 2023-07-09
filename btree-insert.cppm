export module btree:insert;
import :db;

namespace btree {
export template <typename Tp> bool insert(db::nnid *r, db::nnid y, Tp v) {
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

  // db::current()->insert_item(s, y, v);
  return true;
}
} // namespace btree
