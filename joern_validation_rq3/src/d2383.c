  DidFinishNavigationObserver(content::WebContents* web_contents,
                                       bool add_data_reduction_proxy_data)
      : content::WebContentsObserver(web_contents),
        add_data_reduction_proxy_data_(add_data_reduction_proxy_data) {}
