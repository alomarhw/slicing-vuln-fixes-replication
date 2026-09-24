static unsigned numberOfLineBreaks(const String& text)
{
    unsigned length = text.length();
    unsigned count = 0;
    for (unsigned i = 0; i < length; i++) {
        if (text[i] == '\n')
            count++;
    }
    return count;
}
