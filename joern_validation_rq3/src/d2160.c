test_bson_append_oid (void)
{
   bson_oid_t oid;
   bson_t *b;
   bson_t *b2;

   bson_oid_init_from_string (&oid, "1234567890abcdef1234abcd");

   b = bson_new ();
   BSON_ASSERT (bson_append_oid (b, "oid", -1, &oid));
   b2 = get_bson ("test22.bson");
   BSON_ASSERT_BSON_EQUAL (b, b2);
   bson_destroy (b);
   bson_destroy (b2);
}
