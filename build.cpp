#include "../rng/build.hpp"
#include "build.hpp"

int main(int argc, char **argv) {
  using namespace ecow;

  auto poc = unit::create<tool>("poc");
  poc->add_wsdep("rng", rng());
  poc->add_wsdep("silog", silog());
  poc->add_ref(btree());
  poc->add_unit<mod>("poc");

  auto all = unit::create<seq>("all");
  all->add_ref(poc);
  all->add_unit<sys>(poc->executable().string());
  return run_main(all, argc, argv);
}
