static AtomicString makeVisibleEmptyValue(const Vector<String>& symbols)
{
    unsigned maximumLength = 0;
    for (unsigned index = 0; index < symbols.size(); ++index)
        maximumLength = std::max(maximumLength, numGraphemeClusters(symbols[index]));
    StringBuilder builder;
    builder.reserveCapacity(maximumLength);
    for (unsigned length = 0; length < maximumLength; ++length)
        builder.append('-');
    return builder.toAtomicString();
}
