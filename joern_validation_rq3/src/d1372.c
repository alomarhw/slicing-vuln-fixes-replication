netdutils::Status XfrmController::allocateSpi(const XfrmSaInfo& record, uint32_t minSpi,
 uint32_t maxSpi, uint32_t* outSpi,
 const XfrmSocket& sock) {
    xfrm_userspi_info spiInfo{};

 enum { NLMSG_HDR, USERSAID, USERSAID_PAD };

    std::vector<iovec> iov = {
 {NULL, 0}, // reserved for the eventual addition of a NLMSG_HDR
 {&spiInfo, 0}, // main userspi_info struct
 {kPadBytes, 0}, // up to NLMSG_ALIGNTO pad bytes of padding
 };

 int len;
 if (fillUserSaInfo(record, &spiInfo.info) == 0) {
        ALOGE("Failed to fill transport SA Info");
 }

    len = iov[USERSAID].iov_len = sizeof(spiInfo);
    iov[USERSAID_PAD].iov_len = NLMSG_ALIGN(len) - len;

 RandomSpi spiGen = RandomSpi(minSpi, maxSpi);
 int spi;
    netdutils::Status ret;
 while ((spi = spiGen.next()) != INVALID_SPI) {
        spiInfo.min = spi;
        spiInfo.max = spi;
        ret = sock.sendMessage(XFRM_MSG_ALLOCSPI, NETLINK_REQUEST_FLAGS, 0, &iov);

 /* If the SPI is in use, we'll get ENOENT */
 if (netdutils::equalToErrno(ret, ENOENT))
 continue;

 if (isOk(ret)) {
 *outSpi = spi;
            ALOGD("Allocated an SPI: %x", *outSpi);
 } else {
 *outSpi = INVALID_SPI;
            ALOGE("SPI Allocation Failed with error %d", ret.code());
 }

 return ret;
 }

 return ret;
}
