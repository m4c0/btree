export module btree:retrieve;
import :db;

namespace btree {
constexpr db::nnid retrieve(db::storage *dbs, db::nnid r, db::nnid y,
                            db::nnid *s) {
  db::nnid p = r;
  *s = {};

  while (p) {
    *s = p;

    auto node = dbs->read(p);
    if (y < node.k[0].xi) {
      p = node.p0;
      continue;
    }

    unsigned i;
    for (i = 0U; i < node.size; i++) {
      auto xi = node.k[i].xi;
      auto xi1 = node.k[i + 1].xi;
      if (y == xi) {
        return node.k[i].ai;
      }
      if (y < xi) {
        break;
      }
    }
    p = node.k[i - 1].pi;
  }
  return {};
}
} // namespace btree
