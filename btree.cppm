export module btree;
export import :db;
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

  bool insert(db::nnid y, Tp v) {
    db::nnid s{};
    auto r = retrieve<Tp>(m_root, y, &s);
    if (r)
      return false;
    if (!s) {
      m_root = db::current()->create_node({}, true);
      s = m_root;
    }

    auto node = db::current()->read<Tp>(s);
    if (node.size == db::node_limit) {
      // split
    }

    db::current()->add_item(s, y, v);
    return true;
  }
};
}; // namespace btree
