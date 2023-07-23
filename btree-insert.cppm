export module btree:insert;
import :db;
import :retrieve;

namespace btree {
constexpr auto &insert_entry_in_p(db::storage *dbs, db::nnid s, db::key k) {
  auto &node = dbs->read(s);
  for (auto i = 0; i < node.size; i++) {
    if (k.xi < node.k[i].xi) {
      dbs->insert_entry(s, i, k);
      return node;
    }
  }
  dbs->insert_entry(s, node.size, k);
  return node;
}

constexpr bool insert(db::storage *dbs, db::nnid *r, db::nnid y, db::nnid v) {
  db::nnid s{};
  if (retrieve(dbs, *r, y, &s))
    return false;

  if (!s) {
    *r = dbs->create_node({}, true);
    dbs->insert_entry(*r, 0, db::key{y, v});
    return true;
  }

  db::nnid p;
  db::nnid p1;
  db::key k{y, v};
  while (true) {
    auto &n = insert_entry_in_p(dbs, s, k);
    if (n.size < db::node_limit + 1)
      return true;

    p = s;
    p1 = dbs->create_node(n.parent, n.leaf);
    auto &node = dbs->read(s);
    for (auto i = 0; i < db::node_lower_limit; i++) {
      auto key = node.k[i + db::node_lower_limit + 1];
      dbs->insert_entry(p1, i, key);
      if (key.pi)
        dbs->set_parent(key.pi, p1);
    }
    auto p1p0 = node.k[db::node_lower_limit].pi;
    dbs->set_p0(p1, p1p0);
    if (p1p0)
      dbs->set_parent(p1p0, p1);
    dbs->set_size(p, db::node_lower_limit);

    k = node.k[db::node_lower_limit];
    k.pi = p1;

    auto q = node.parent;
    if (!q)
      break;

    s = q;
  }

  *r = dbs->create_node({}, false);
  dbs->set_p0(*r, p);
  dbs->insert_entry(*r, 0, k);
  dbs->set_parent(p, *r);
  dbs->set_parent(p1, *r);
  return true;
}
} // namespace btree
