raptor_turtle_writer_newline(raptor_turtle_writer *turtle_writer)
{
  int num_spaces;
  
  raptor_iostream_write_byte('\n', turtle_writer->iostr);
 
  if(!TURTLE_WRITER_AUTO_INDENT(turtle_writer))
    return;

  num_spaces = turtle_writer->depth * turtle_writer->indent;

  while(num_spaces > 0) {
    int count;
    count = (num_spaces > RAPTOR_GOOD_CAST(int, SPACES_BUFFER_SIZE)) ? 
            RAPTOR_GOOD_CAST(int, SPACES_BUFFER_SIZE) : num_spaces;

    raptor_iostream_counted_string_write(spaces_buffer, count, turtle_writer->iostr);

    num_spaces -= count;
  }

  return;
}
