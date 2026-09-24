static void png_put_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length != 0)
    {
      png_size_t
        check;

      check=(png_size_t) WriteBlob(image,(size_t) length,data);

      if (check != length)
        png_error(png_ptr,"WriteBlob Failed");
    }
}
