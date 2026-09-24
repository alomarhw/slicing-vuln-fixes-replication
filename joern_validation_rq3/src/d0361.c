static int do_spice_init(SpiceCoreInterface *core_interface)
{
    spice_info("starting %s", version_string);

    if (core_interface->base.major_version != SPICE_INTERFACE_CORE_MAJOR) {
        spice_warning("bad core interface version");
        goto err;
    }
    core = core_interface;
    reds->listen_socket = -1;
    reds->secure_listen_socket = -1;
    init_vd_agent_resources();
    ring_init(&reds->clients);
    reds->num_clients = 0;
    main_dispatcher_init(core);
    ring_init(&reds->channels);
    ring_init(&reds->mig_target_clients);
    ring_init(&reds->char_devs_states);
    ring_init(&reds->mig_wait_disconnect_clients);
    reds->vm_running = TRUE; /* for backward compatibility */

    if (!(reds->mig_timer = core->timer_add(migrate_timeout, NULL))) {
        spice_error("migration timer create failed");
    }

#ifdef RED_STATISTICS
    int shm_name_len;
    int fd;

    shm_name_len = strlen(SPICE_STAT_SHM_NAME) + 20;
    reds->stat_shm_name = (char *)spice_malloc(shm_name_len);
    snprintf(reds->stat_shm_name, shm_name_len, SPICE_STAT_SHM_NAME, getpid());
    if ((fd = shm_open(reds->stat_shm_name, O_CREAT | O_RDWR, 0444)) == -1) {
        spice_error("statistics shm_open failed, %s", strerror(errno));
    }
    if (ftruncate(fd, REDS_STAT_SHM_SIZE) == -1) {
        spice_error("statistics ftruncate failed, %s", strerror(errno));
    }
    reds->stat = (SpiceStat *)mmap(NULL, REDS_STAT_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (reds->stat == (SpiceStat *)MAP_FAILED) {
        spice_error("statistics mmap failed, %s", strerror(errno));
    }
    memset(reds->stat, 0, REDS_STAT_SHM_SIZE);
    reds->stat->magic = SPICE_STAT_MAGIC;
    reds->stat->version = SPICE_STAT_VERSION;
    reds->stat->root_index = INVALID_STAT_REF;
    if (pthread_mutex_init(&reds->stat_lock, NULL)) {
        spice_error("mutex init failed");
    }
#endif

    if (!(reds->mm_timer = core->timer_add(mm_timer_proc, NULL))) {
        spice_error("mm timer create failed");
    }
    reds_enable_mm_timer();

    if (reds_init_net() < 0) {
        goto err;
    }
    if (reds->secure_listen_socket != -1) {
        if (reds_init_ssl() < 0) {
            goto err;
        }
    }
#if HAVE_SASL
    int saslerr;
    if ((saslerr = sasl_server_init(NULL, sasl_appname ?
                                    sasl_appname : "spice")) != SASL_OK) {
        spice_error("Failed to initialize SASL auth %s",
                  sasl_errstring(saslerr, NULL, NULL));
        goto err;
    }
#endif

    reds->main_channel = main_channel_init();
    inputs_init();

    reds->mouse_mode = SPICE_MOUSE_MODE_SERVER;

    reds_client_monitors_config_cleanup();

    reds->allow_multiple_clients = getenv(SPICE_DEBUG_ALLOW_MC_ENV) != NULL;
    if (reds->allow_multiple_clients) {
        spice_warning("spice: allowing multiple client connections (crashy)");
    }
    atexit(reds_exit);
    return 0;

err:
    return -1;
}
