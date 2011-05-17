#include "rb-grn.h"

VALUE rb_cGrnIndexCursor;

VALUE
rb_grn_index_cursor_to_ruby_object (grn_ctx *context,
				    grn_obj *cursor,
				    grn_bool owner)
{
  return GRNOBJECT2RVAL(rb_cGrnIndexCursor, context, cursor, owner);
}

void
rb_grn_init_index_cursor (VALUE mGrn)
{
  rb_cGrnIndexCursor = rb_define_class_under(mGrn, "IndexCursor", rb_cObject);
  rb_define_alloc_func(rb_cGrnIndexCursor, rb_grn_object_alloc);
}
