load_identity_file(Identity *id)
{
	Key *private = NULL;
	char prompt[300], *passphrase, *comment;
	int r, perm_ok = 0, quit = 0, i;
	struct stat st;

	if (stat(id->filename, &st) < 0) {
		(id->userprovided ? logit : debug3)("no such identity: %s: %s",
		    id->filename, strerror(errno));
		return NULL;
	}
	snprintf(prompt, sizeof prompt,
	    "Enter passphrase for key '%.100s': ", id->filename);
	for (i = 0; i <= options.number_of_password_prompts; i++) {
		if (i == 0)
			passphrase = "";
		else {
			passphrase = read_passphrase(prompt, 0);
			if (*passphrase == '\0') {
				debug2("no passphrase given, try next key");
				free(passphrase);
				break;
			}
		}
		switch ((r = sshkey_load_private_type(KEY_UNSPEC, id->filename,
		    passphrase, &private, &comment, &perm_ok))) {
		case 0:
			break;
		case SSH_ERR_KEY_WRONG_PASSPHRASE:
			if (options.batch_mode) {
				quit = 1;
				break;
			}
			if (i != 0)
				debug2("bad passphrase given, try again...");
			break;
		case SSH_ERR_SYSTEM_ERROR:
			if (errno == ENOENT) {
				debug2("Load key \"%s\": %s",
				    id->filename, ssh_err(r));
				quit = 1;
				break;
			}
			/* FALLTHROUGH */
		default:
			error("Load key \"%s\": %s", id->filename, ssh_err(r));
			quit = 1;
			break;
		}
		if (!quit && private != NULL && id->agent_fd == -1 &&
		    !(id->key && id->isprivate))
			maybe_add_key_to_agent(id->filename, private, comment,
			    passphrase);
		if (i > 0) {
			explicit_bzero(passphrase, strlen(passphrase));
			free(passphrase);
		}
		free(comment);
		if (private != NULL || quit)
			break;
	}
	return private;
}
