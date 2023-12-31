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
  explicit constexpr tree(db::storage *s, db::nnid r)
      : m_storage{s}, m_root{r} {}

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }
  constexpr void set_root(db::nnid r) noexcept { m_root = r; }

  [[nodiscard]] constexpr auto get(db::nnid y) const {
    db::nnid s{};
    return retrieve(m_storage, m_root, y, &s);
  }
  [[nodiscard]] constexpr bool has(db::nnid y) const { return !!get(y); }

  constexpr bool insert(db::nnid y, db::nnid v) {
    return btree::insert(m_storage, &m_root, y, v);
  }

  constexpr bool remove(db::nnid y) {
    return btree::remove(m_storage, &m_root, y);
  }
};
} // namespace btree

module :private;

// Just testing if btree is constexpr-able. Real tests are in POC
static_assert([] {
  using namespace btree;

  db::storage s{};
  tree t{&s};

  t.insert(db::nnid{99}, db::nnid{11});
  t.remove(db::nnid{99});

  return !t.has(db::nnid{99});
}());
