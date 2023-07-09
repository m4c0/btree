export module btree:insert;
import :db;
export import :log;

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
    log("creating root %d", r->index());
    db::current()->insert_entry(*r, 0, db::key<Tp>{y, v});
    return true;
  }

  auto &node = db::current()->read<Tp>(s);
  insert_entry_in_p(s, node, db::key<Tp>{y, v});

  if (node.size == db::node_limit + 1) {
    log("spliting %d", s.index());
    auto p = s;
    auto p1 = db::current()->create_node(node.parent, node.leaf);
    for (auto i = 0; i < db::node_lower_limit; i++) {
      auto key = node.k[i + db::node_lower_limit + 1];
      db::current()->insert_entry(p1, i, key);
    }
    db::current()->set_p0(p1, node.k[db::node_lower_limit].pi);
    db::current()->set_size(p, db::node_lower_limit);

    auto k = node.k[db::node_lower_limit];
    k.pi = p1;

    auto q = node.parent;
    if (q) {
      auto &qnode = db::current()->read<Tp>(q);
      if (qnode.size == db::node_limit)
        throw 0;

      insert_entry_in_p(q, qnode, k);
      db::current()->set_parent(p1, q);
      return true;
    }

    *r = db::current()->create_node({}, false);
    log("new root %d", r->index());
    db::current()->set_p0(*r, p);
    db::current()->insert_entry(*r, 0, k);
    db::current()->set_parent(p, *r);
    db::current()->set_parent(p1, *r);
  }

  return true;
}
} // namespace btree
