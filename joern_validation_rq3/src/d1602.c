static void megasas_command_complete(SCSIRequest *req, uint32_t status,
                                     size_t resid)
{
    MegasasCmd *cmd = req->hba_private;
    uint8_t cmd_status = MFI_STAT_OK;

    trace_megasas_command_complete(cmd->index, status, resid);

    if (cmd->req != req) {
        /*
         * Internal command complete
         */
        cmd_status = megasas_finish_internal_command(cmd, req, resid);
        if (cmd_status == MFI_STAT_INVALID_STATUS) {
            return;
        }
    } else {
        req->status = status;
        trace_megasas_scsi_complete(cmd->index, req->status,
                                    cmd->iov_size, req->cmd.xfer);
        if (req->status != GOOD) {
            cmd_status = MFI_STAT_SCSI_DONE_WITH_ERROR;
        }
        if (req->status == CHECK_CONDITION) {
            megasas_copy_sense(cmd);
        }

        megasas_unmap_sgl(cmd);
        cmd->frame->header.scsi_status = req->status;
        scsi_req_unref(cmd->req);
        cmd->req = NULL;
    }
    cmd->frame->header.cmd_status = cmd_status;
    megasas_unmap_frame(cmd->state, cmd);
    megasas_complete_frame(cmd->state, cmd->context);
}
