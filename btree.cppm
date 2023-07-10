export module btree;
export import :db;
import :insert;
import :retrieve;
import :remove;

namespace btree {
export class tree {
  // Eventually, storage will be platform-dependent, therefore, let's leave it
  // outside the tree for now
  db::storage *m_storage;
  db::nnid m_root{};

public:
  explicit constexpr tree(db::storage *s) : m_storage{s} {}

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }

  [[nodiscard]] constexpr auto get(db::nnid y) {
    db::nnid s{};
    return retrieve(m_storage, m_root, y, &s);
  }
  [[nodiscard]] constexpr bool has(db::nnid y) { return !!get(y); }

  constexpr bool insert(db::nnid y, db::nnid v) {
    return btree::insert(m_storage, &m_root, y, v);
  }

  constexpr bool remove(db::nnid y) {
    return btree::remove(m_storage, &m_root, y);
  }
};
} // namespace btree

module :private;

static_assert([] {
  btree::db::storage s{};
  btree::tree t{&s};

  return true;
}());
