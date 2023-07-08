export module btree:db;
import hai;
import silog;

export namespace btree::db {
constexpr const auto node_lower_limit = 4;
constexpr const auto node_limit = 16;

class inconsistency_error {};

class nnid {
  unsigned m_id{};

public:
  constexpr explicit nnid() = default;
  constexpr explicit nnid(unsigned v) : m_id{v + 1} {}

  explicit constexpr operator bool() const noexcept { return m_id > 0; }
  constexpr unsigned index() const noexcept { return m_id - 1; }

  constexpr bool operator==(const nnid &o) const noexcept {
    return m_id == o.m_id;
  }
};

struct node {
  nnid parent{};
  bool leaf{};
  bool in_use{};
  // unsigned size{};
  // link children[node_limit]{};
};

class storage {
  static constexpr const auto initial_cap = 128;
  static constexpr const auto resize_cap = 128;

  hai::array<node> m_nodes{initial_cap};

  [[nodiscard]] node &get(nnid id) {
    unsigned idx = id.index();
    if (idx >= m_nodes.size()) {
      silog::log(silog::error, "attempt of reading node past end: %d", idx);
      throw inconsistency_error();
    }

    node &res = m_nodes[idx];
    if (!res.in_use) {
      silog::log(silog::error, "attempt of reading node not in use: %d", idx);
      throw inconsistency_error();
    }
    return res;
  }

  [[nodiscard]] nnid find_unused_node() {
    for (auto i = 0U; i < m_nodes.size(); i++) {
      auto &n = m_nodes[i];
      if (!n.in_use) {
        return nnid{i};
      }
    }
    auto i = m_nodes.size();
    m_nodes.add_capacity(resize_cap);
    return nnid{i};
  }

public:
  [[nodiscard]] const node &read(nnid id) { return get(id); }

  [[nodiscard]] nnid create_node(nnid p, bool leaf) {
    auto res = find_unused_node();
    auto &n = m_nodes[res.index()];
    n.parent = p;
    n.leaf = leaf;
    n.in_use = true;
    return res;
  }
  void delete_node(nnid n) { get(n) = {}; }

  void set_parent(nnid n, nnid p) { get(n).parent = p; }
};

storage *&current() noexcept;

} // namespace btree::db
