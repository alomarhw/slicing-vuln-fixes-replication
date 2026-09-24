test_bson_build_child_deep_no_begin_end (void)
{
   union {
      bson_t b;
      bson_impl_alloc_t a;
   } u;

   int count = 0;

   bson_init (&u.b);
   test_bson_build_child_deep_no_begin_end_1 (&u.b, &count);
   BSON_ASSERT (bson_validate (&u.b, BSON_VALIDATE_NONE, NULL));
   BSON_ASSERT (u.a.alloclen == 1024);
   BSON_ASSERT_BSON_EQUAL_FILE (&u.b, "test39.bson");
   bson_destroy (&u.b);
}
