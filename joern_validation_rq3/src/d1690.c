static int handle_NPP_Print(rpc_connection_t *connection)
{
  D(bug("handle_NPP_Print\n"));

  PluginInstance *plugin;
  NPPrint printInfo;
  uint32_t platform_print_id;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_UINT32, &platform_print_id,
								  RPC_TYPE_NP_PRINT, &printInfo,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Print() get args", error);
	return error;
  }

  NPPrintCallbackStruct printer;
  printer.type = NP_PRINT;
  printer.fp = platform_print_id ? tmpfile() : NULL;
  switch (printInfo.mode) {
  case NP_FULL:
	printInfo.print.fullPrint.platformPrint = &printer;
	break;
  case NP_EMBED:
	printInfo.print.embedPrint.platformPrint = &printer;
	create_window_attributes(printInfo.print.embedPrint.window.ws_info);
	break;
  }

  g_NPP_Print(PLUGIN_INSTANCE_NPP(plugin), &printInfo);

  if (printer.fp) {
	long file_size = ftell(printer.fp);
	D(bug(" writeback data [%d bytes]\n", file_size));
	rewind(printer.fp);
	if (file_size > 0) {
	  NPPrintData printData;
	  const int printDataMaxSize = sizeof(printData.data);
	  int n = file_size / printDataMaxSize;
	  while (--n >= 0) {
		printData.size = printDataMaxSize;
		if (fread(&printData.data, sizeof(printData.data), 1, printer.fp) != 1) {
		  npw_printf("ERROR: unexpected end-of-file or error condition in NPP_Print\n");
		  break;
		}
		npw_plugin_instance_ref(plugin);
		invoke_NPN_PrintData(plugin, platform_print_id, &printData);
		npw_plugin_instance_unref(plugin);
	  }
	  printData.size = file_size % printDataMaxSize;
	  if (fread(&printData.data, printData.size, 1, printer.fp) != 1)
		npw_printf("ERROR: unexpected end-of-file or error condition in NPP_Print\n");
	  npw_plugin_instance_ref(plugin);
	  invoke_NPN_PrintData(plugin, platform_print_id, &printData);
	  npw_plugin_instance_unref(plugin);
	}
	fclose(printer.fp);
  }

  if (printInfo.mode == NP_EMBED) {
	NPWindow *window = &printInfo.print.embedPrint.window;
	if (window->ws_info) {
	  destroy_window_attributes(window->ws_info);
	  window->ws_info = NULL;
	}
  }

  uint32_t plugin_printed = FALSE;
  if (printInfo.mode == NP_FULL)
	plugin_printed = printInfo.print.fullPrint.pluginPrinted;
  return rpc_method_send_reply(connection, RPC_TYPE_BOOLEAN, plugin_printed, RPC_TYPE_INVALID);
}
