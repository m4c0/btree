export module btree;
export import :db;
import :insert;
import :retrieve;
import :remove;

namespace btree {
export class tree {
  db::nnid m_root{};

public:
  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }

  [[nodiscard]] auto get(db::nnid y) const {
    db::nnid s{};
    return retrieve(m_root, y, &s);
  }
  [[nodiscard]] bool has(db::nnid y) const { return !!get(y); }

  bool insert(db::nnid y, db::nnid v) { return btree::insert(&m_root, y, v); }

  bool remove(db::nnid y) { return btree::remove(&m_root, y); }
};
}; // namespace btree
