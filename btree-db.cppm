export module btree:db;
import hai;
import silog;

export namespace btree::db {
constexpr const auto node_limit = 16;

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
  nnid pi{};
};

template <typename Tp> struct node;
template <> struct node<void> {
  nnid parent{};
  bool leaf{};
  bool in_use{};
  unsigned size{};
  nnid p0{};
  key k[node_limit + 1]{};
};
template <typename Tp> struct node : node<void> {
  Tp ai[node_limit + 1]{};
};

template <typename Tp> class alpha_storage;
template <> class alpha_storage<void> {
public:
  virtual ~alpha_storage() = default;

  [[nodiscard]] virtual node<void> &get(nnid id, bool in_use) = 0;
  [[nodiscard]] virtual nnid find_unused_node() = 0;
};
template <typename Tp>
class alpha_storage : public alpha_storage<void>, public hai::array<node<Tp>> {
  static constexpr const auto initial_cap = 128;
  static constexpr const auto resize_cap = 128;
  using bro = hai::array<node<Tp>>;

public:
  using bro::bro;

  [[nodiscard]] node<void> &get(nnid id, bool in_use) override {
    unsigned idx = id.index();
    if (idx >= this->size()) {
      silog::log(silog::error, "attempt of reading node past end: %d", idx);
      throw inconsistency_error();
    }

    auto &res = (*this)[idx];
    if (res.in_use != in_use) {
      silog::log(silog::error, "attempt of reading node not in use: %d", idx);
      throw inconsistency_error();
    }
    return res;
  }

  [[nodiscard]] nnid find_unused_node() override {
    for (auto i = 0U; i < this->size(); i++) {
      auto &n = (*this)[i];
      if (!n.in_use) {
        return nnid{i};
      }
    }
    auto i = this->size();
    this->add_capacity(resize_cap);
    return nnid{i};
  }
};

class storage {
  hai::uptr<alpha_storage<void>> m_nodes;

  template <typename Tp> [[nodiscard]] node<Tp> &get(nnid id) {
    return static_cast<node<Tp> &>(m_nodes->get(id, true));
  }

public:
  template <typename Tp> storage(Tp) : m_nodes{new alpha_storage<Tp>()} {}

  template <typename Tp> [[nodiscard]] const node<Tp> &read(nnid id) {
    return get<Tp>(id);
  }

  [[nodiscard]] nnid create_node(nnid p, bool leaf) {
    auto res = m_nodes->find_unused_node();
    auto &n = m_nodes->get(res, false);
    n.parent = p;
    n.leaf = leaf;
    n.in_use = true;
    return res;
  }
  void delete_node(nnid n) { m_nodes->get(n, true) = {}; }

  void set_parent(nnid n, nnid p) { m_nodes->get(n, true).parent = p; }

  template <typename Tp> void add_item(nnid p, nnid x, Tp v) {
    silog::log(silog::debug, "add item %d %d", p.index(), x.index());
    auto &node = get<Tp>(p);
    if (!node.leaf) {
      silog::log(silog::error, "non-leaf insertion");
      throw inconsistency_error{};
    }
    if (node.size == node_limit) {
      silog::log(silog::error, "btree node overflow");
      throw inconsistency_error{};
    }
    node.k[node.size].xi = x;
    node.ai[node.size] = v;
    node.size++;
  }
};

storage *&current() noexcept;

} // namespace btree::db
