		void CWebServer::Cmd_GetCustomIconSet(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetCustomIconSet";
			int ii = 0;
			for (const auto & itt : m_custom_light_icons)
			{
				if (itt.idx >= 100)
				{
					std::string IconFile16 = "images/" + itt.RootFile + ".png";
					std::string IconFile48On = "images/" + itt.RootFile + "48_On.png";
					std::string IconFile48Off = "images/" + itt.RootFile + "48_Off.png";

					root["result"][ii]["idx"] = itt.idx - 100;
					root["result"][ii]["Title"] = itt.Title;
					root["result"][ii]["Description"] = itt.Description;
					root["result"][ii]["IconFile16"] = IconFile16;
					root["result"][ii]["IconFile48On"] = IconFile48On;
					root["result"][ii]["IconFile48Off"] = IconFile48Off;
					ii++;
				}
			}
		}
