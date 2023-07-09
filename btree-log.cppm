export module btree:log;
import silog;

namespace btree {
export inline void log(const char *msg, auto... args) {
  silog::log(silog::debug, msg, args...);
}
} // namespace btree
