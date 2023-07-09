export module btree;
export import :db;
export import :insert;
export import :retrieve;
export import :remove;
export import missingno;

namespace btree {
export template <typename Tp> class tree {
  db::nnid m_root{};

public:
  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }

  [[nodiscard]] mno::opt<Tp> get(db::nnid y) const {
    db::nnid s{};
    return retrieve<Tp>(m_root, y, &s);
  }
  [[nodiscard]] bool has(db::nnid y) const { return !!get(y); }

  bool insert(db::nnid y, Tp v) { return btree::insert(&m_root, y, v); }

  bool remove(db::nnid y) { return btree::remove(&m_root, y); }
};
}; // namespace btree
