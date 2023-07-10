#pragma once
#include "../ecow/ecow.hpp"
#include "../hai/build.hpp"
#include "../silog/build.hpp"

auto btree() {
  using namespace ecow;

  auto m = unit::create<mod>("btree");
  m->add_wsdep("hai", hai());
  m->add_wsdep("silog", silog());
  m->add_part("log");
  m->add_part("db");
  m->add_part("retrieve");
  m->add_part("insert");
  m->add_part("remove");
  m->add_impl("singleton");

  return m;
}
