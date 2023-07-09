export module btree:insert;
import :db;
import :retrieve;

export namespace btree {
template <typename Tp> auto &insert_entry_in_p(db::nnid s, db::key<Tp> k) {
  auto &node = db::current()->read<Tp>(s);
  for (auto i = 0; i < node.size; i++) {
    if (k.xi < node.k[i].xi) {
      db::current()->insert_entry(s, i, k);
      return node;
    }
  }
  db::current()->insert_entry(s, node.size, k);
  return node;
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

  db::nnid p;
  db::nnid p1;
  db::key<Tp> k{y, v};
  while (true) {
    auto &node = insert_entry_in_p(s, k);
    if (node.size < db::node_limit + 1)
      return true;

    p = s;
    p1 = db::current()->create_node(node.parent, node.leaf);
    for (auto i = 0; i < db::node_lower_limit; i++) {
      auto key = node.k[i + db::node_lower_limit + 1];
      db::current()->insert_entry(p1, i, key);
      if (key.pi)
        db::current()->set_parent(key.pi, p1);
    }
    auto p1p0 = node.k[db::node_lower_limit].pi;
    db::current()->set_p0(p1, p1p0);
    if (p1p0)
      db::current()->set_parent(p1p0, p1);
    db::current()->set_size(p, db::node_lower_limit);

    k = node.k[db::node_lower_limit];
    k.pi = p1;

    auto q = node.parent;
    if (!q)
      break;

    s = q;
  }

  *r = db::current()->create_node({}, false);
  db::current()->set_p0(*r, p);
  db::current()->insert_entry(*r, 0, k);
  db::current()->set_parent(p, *r);
  db::current()->set_parent(p1, *r);
  return true;
}
} // namespace btree
