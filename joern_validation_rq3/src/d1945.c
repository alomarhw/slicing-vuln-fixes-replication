enum act_parse_ret parse_set_req_line(const char **args, int *orig_arg, struct proxy *px,
                                      struct act_rule *rule, char **err)
{
	int cur_arg = *orig_arg;

	rule->action = ACT_CUSTOM;

	switch (args[0][4]) {
	case 'm' :
		rule->arg.http.action = 0;
		rule->action_ptr = http_action_set_req_line;
		break;
	case 'p' :
		rule->arg.http.action = 1;
		rule->action_ptr = http_action_set_req_line;
		break;
	case 'q' :
		rule->arg.http.action = 2;
		rule->action_ptr = http_action_set_req_line;
		break;
	case 'u' :
		rule->arg.http.action = 3;
		rule->action_ptr = http_action_set_req_line;
		break;
	default:
		memprintf(err, "internal error: unhandled action '%s'", args[0]);
		return ACT_RET_PRS_ERR;
	}

	if (!*args[cur_arg] ||
	    (*args[cur_arg + 1] && strcmp(args[cur_arg + 1], "if") != 0 && strcmp(args[cur_arg + 1], "unless") != 0)) {
		memprintf(err, "expects exactly 1 argument <format>");
		return ACT_RET_PRS_ERR;
	}

	LIST_INIT(&rule->arg.http.logfmt);
	px->conf.args.ctx = ARGC_HRQ;
	if (!parse_logformat_string(args[cur_arg], px, &rule->arg.http.logfmt, LOG_OPT_HTTP,
	                            (px->cap & PR_CAP_FE) ? SMP_VAL_FE_HRQ_HDR : SMP_VAL_BE_HRQ_HDR, err)) {
		return ACT_RET_PRS_ERR;
	}

	(*orig_arg)++;
	return ACT_RET_PRS_OK;
}
