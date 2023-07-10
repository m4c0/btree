export module btree:db;
export import hai;
import silog;

export namespace btree::db {
constexpr const auto node_lower_limit = 8;
constexpr const auto node_limit = node_lower_limit * 2;

class inconsistency_error {};

class nnid {
  unsigned m_id{};

public:
  constexpr nnid() = default;
  constexpr explicit nnid(unsigned v) : m_id{v + 1} {}

  explicit constexpr operator bool() const noexcept { return m_id > 0; }
  constexpr unsigned index() const noexcept { return m_id - 1; }

  constexpr bool operator<(const nnid &o) const noexcept {
    return m_id < o.m_id;
  }
  constexpr bool operator==(const nnid &o) const noexcept {
    return m_id == o.m_id;
  }
};

struct key {
  nnid xi{};
  nnid ai{};
  nnid pi{};
};

struct node {
  nnid parent{};
  bool leaf{};
  bool in_use{};
  unsigned size{};
  nnid p0{};
  key k[node_limit + 1]{};
};

class storage {
  static constexpr const auto initial_cap = 128;
  static constexpr const auto resize_cap = 128;

  hai::array<node> m_nodes{initial_cap};

  [[nodiscard]] constexpr node &get(nnid id) { return get(id, true); }
  [[nodiscard]] constexpr node &get(nnid id, bool in_use) {
    unsigned idx = id.index();
    if (idx >= m_nodes.size()) {
      silog::log(silog::error, "attempt of reading node past end: %d", idx);
      throw inconsistency_error();
    }

    auto &res = m_nodes[idx];
    if (res.in_use != in_use) {
      silog::log(silog::error, "attempt of reading node not in use: %d", idx);
      throw inconsistency_error();
    }
    return res;
  }

  [[nodiscard]] constexpr nnid find_unused_node() {
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
  [[nodiscard]] constexpr const node &read(nnid id) { return get(id); }

  [[nodiscard]] constexpr nnid create_node(nnid p, bool leaf) {
    auto res = find_unused_node();
    auto &n = get(res, false);
    n.parent = p;
    n.leaf = leaf;
    n.in_use = true;
    return res;
  }
  constexpr void delete_node(nnid n) { get(n) = {}; }

  constexpr void set_parent(nnid n, nnid p) { get(n).parent = p; }
  constexpr void set_p0(nnid p, nnid p0) { get(p).p0 = p0; }
  constexpr void set_size(nnid p, unsigned s) { get(p).size = s; }

  constexpr void insert_entry(nnid p, unsigned idx, key k) {
    auto &node = get(p);
    for (auto i = node.size; i > idx; i--) {
      node.k[i] = node.k[i - 1];
    }
    node.k[idx] = k;
    node.size++;
  }
  constexpr void append_entry(nnid p, key k) {
    auto &node = get(p);
    node.k[node.size++] = k;
  }
  constexpr void set_entry(nnid p, unsigned idx, key k) {
    auto &node = get(p);
    node.k[idx] = k;
  }

  constexpr auto remove_entry(nnid p, unsigned idx) {
    auto &node = get(p);
    for (auto i = idx; i < node.size; i++) {
      node.k[i] = node.k[i + 1];
    }
    node.size--;
    return node.size;
  }
};

} // namespace btree::db
