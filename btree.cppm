export module btree;
export import :db;

namespace btree {
export template <typename Tp> class tree {
  db::nnid m_root{};

  bool retrieve(db::nnid y, db::nnid *s) const {
    db::nnid p = m_root;
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
          return true;
        }
        if (xi < y && y < xi1) {
          p = node.k[i].pi;
          goto lbl;
        }
      }
      p = node.k[node.size].pi;
    }
    return false;
  }

public:
  bool has(db::nnid y) const {
    db::nnid s{};
    return retrieve(y, &s);
  }
};
}; // namespace btree
