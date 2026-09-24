  explicit TestFinishObserver(RenderViewHost* render_view_host, int timeout_s)
      : finished_(false), waiting_(false), timeout_s_(timeout_s) {
    registrar_.Add(this, content::NOTIFICATION_DOM_OPERATION_RESPONSE,
                   content::Source<RenderViewHost>(render_view_host));
    timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(timeout_s),
                 this, &TestFinishObserver::OnTimeout);
  }
