static __inline VOID CalculateTcpChecksumGivenPseudoCS(TCPHeader *pTcpHeader, tCompletePhysicalAddress *pDataPages, ULONG ulStartOffset, ULONG tcpLength)
{
    pTcpHeader->tcp_xsum = CheckSumCalculator(pDataPages, ulStartOffset, tcpLength);
}
