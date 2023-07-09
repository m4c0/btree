export module btree;
export import :db;
export import :insert;
export import :retrieve;
export import missingno;

namespace btree {
export template <typename Tp> class tree {
  db::nnid m_root{};

public:
  [[nodiscard]] mno::opt<Tp> get(db::nnid y) const {
    db::nnid s{};
    return retrieve<Tp>(m_root, y, &s);
  }
  [[nodiscard]] bool has(db::nnid y) const { return !!get(y); }

  bool insert(db::nnid y, Tp v) { return btree::insert(&m_root, y, v); }
};
}; // namespace btree
