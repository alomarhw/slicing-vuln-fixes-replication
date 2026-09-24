int test_div_word(BIO *bp)
{
    BIGNUM a, b;
    BN_ULONG r, s;
    int i;

    BN_init(&a);
    BN_init(&b);

    for (i = 0; i < num0; i++) {
        do {
            BN_bntest_rand(&a, 512, -1, 0);
            BN_bntest_rand(&b, BN_BITS2, -1, 0);
        } while (BN_is_zero(&b));

        s = b.d[0];
        BN_copy(&b, &a);
        r = BN_div_word(&b, s);

        if (bp != NULL) {
            if (!results) {
                BN_print(bp, &a);
                BIO_puts(bp, " / ");
                print_word(bp, s);
                BIO_puts(bp, " - ");
            }
            BN_print(bp, &b);
            BIO_puts(bp, "\n");

            if (!results) {
                BN_print(bp, &a);
                BIO_puts(bp, " % ");
                print_word(bp, s);
                BIO_puts(bp, " - ");
            }
            print_word(bp, r);
            BIO_puts(bp, "\n");
        }
        BN_mul_word(&b, s);
        BN_add_word(&b, r);
        BN_sub(&b, &a, &b);
        if (!BN_is_zero(&b)) {
            fprintf(stderr, "Division (word) test failed!\n");
            return 0;
        }
    }
    BN_free(&a);
    BN_free(&b);
    return (1);
}
