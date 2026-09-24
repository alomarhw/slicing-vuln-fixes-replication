static void report_error(struct translate_ctx *ctx, const char *format, ...)
{
   va_list args;
   int line = 1;
   int column = 1;
   const char *itr = ctx->text;

   debug_printf("\nTGSI asm error: ");

   va_start(args, format);
   _debug_vprintf(format, args);
   va_end(args);

   while (itr != ctx->cur) {
      if (*itr == '\n') {
         column = 1;
         ++line;
      }
      ++column;
      ++itr;
   }

   debug_printf(" [%d : %d] \n", line, column);
}
