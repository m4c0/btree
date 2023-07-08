module btree;
import :db;

btree::db::storage *&btree::db::current() noexcept {
  static storage *i{};
  return i;
}
