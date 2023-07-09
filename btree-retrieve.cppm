export module btree:retrieve;
import :db;
import missingno;

namespace btree {
export template <typename Tp>
mno::opt<Tp> retrieve(db::nnid r, db::nnid y, db::nnid *s) {
  db::nnid p = r;
  *s = {};

lbl:
  while (p) {
    *s = p;

    auto node = db::current()->read<Tp>(p);
    if (y < node.k[0].xi) {
      p = node.p0;
      continue;
    }

    for (auto i = 0; i < node.size; i++) {
      auto xi = node.k[i].xi;
      auto xi1 = node.k[i + 1].xi;
      if (y == xi) {
        return mno::opt<Tp>{node.k[i].ai};
      }
      if (xi < y && y < xi1) {
        p = node.k[i].pi;
        goto lbl;
      }
    }
    p = node.k[node.size - 1].pi;
  }
  return {};
}
} // namespace btree
