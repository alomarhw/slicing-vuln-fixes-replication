static int audit_callback(void* data, security_class_t /* cls */, char* buf, size_t len)
{
 struct debugger_request_t* req = reinterpret_cast<debugger_request_t*>(data);

 if (!req) {
        ALOGE("No debuggerd request audit data");
 return 0;
 }

    snprintf(buf, len, "pid=%d uid=%d gid=%d", req->pid, req->uid, req->gid);
 return 0;
}
