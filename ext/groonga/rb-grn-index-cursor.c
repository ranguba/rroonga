#include "rb-grn.h"

VALUE rb_cGrnIndexCursor;

void
rb_grn_init_index_cursor (VALUE mGrn)
{
  rb_cGrnIndexCursor = rb_define_class_under(mGrn, "IndexCursor", rb_cObject);
  rb_define_alloc_func(rb_cGrnIndexCursor, rb_grn_object_alloc);
}
