void TabStripModel::ExecuteContextMenuCommand(
    int context_index, ContextMenuCommand command_id) {
  DCHECK(command_id > CommandFirst && command_id < CommandLast);
  switch (command_id) {
    case CommandNewTab:
      content::RecordAction(UserMetricsAction("TabContextMenu_NewTab"));
      UMA_HISTOGRAM_ENUMERATION("Tab.NewTab",
                                TabStripModel::NEW_TAB_CONTEXT_MENU,
                                TabStripModel::NEW_TAB_ENUM_COUNT);
      delegate()->AddBlankTabAt(context_index + 1, true);
      break;

    case CommandReload: {
      content::RecordAction(UserMetricsAction("TabContextMenu_Reload"));
      std::vector<int> indices = GetIndicesForCommand(context_index);
      for (size_t i = 0; i < indices.size(); ++i) {
        WebContents* tab = GetWebContentsAt(indices[i]);
        if (tab) {
          CoreTabHelperDelegate* core_delegate =
              CoreTabHelper::FromWebContents(tab)->delegate();
          if (!core_delegate || core_delegate->CanReloadContents(tab))
            tab->GetController().Reload(true);
        }
      }
      break;
    }

    case CommandDuplicate: {
      content::RecordAction(UserMetricsAction("TabContextMenu_Duplicate"));
      std::vector<int> indices = GetIndicesForCommand(context_index);
      std::vector<WebContents*> tabs;
      for (size_t i = 0; i < indices.size(); ++i)
        tabs.push_back(GetWebContentsAt(indices[i]));
      for (size_t i = 0; i < tabs.size(); ++i) {
        int index = GetIndexOfWebContents(tabs[i]);
        if (index != -1 && delegate_->CanDuplicateContentsAt(index))
          delegate_->DuplicateContentsAt(index);
      }
      break;
    }

    case CommandCloseTab: {
      content::RecordAction(UserMetricsAction("TabContextMenu_CloseTab"));
      InternalCloseTabs(GetIndicesForCommand(context_index),
                        CLOSE_CREATE_HISTORICAL_TAB | CLOSE_USER_GESTURE);
      break;
    }

    case CommandCloseOtherTabs: {
      content::RecordAction(
          UserMetricsAction("TabContextMenu_CloseOtherTabs"));
      InternalCloseTabs(GetIndicesClosedByCommand(context_index, command_id),
                        CLOSE_CREATE_HISTORICAL_TAB);
      break;
    }

    case CommandCloseTabsToRight: {
      content::RecordAction(
          UserMetricsAction("TabContextMenu_CloseTabsToRight"));
      InternalCloseTabs(GetIndicesClosedByCommand(context_index, command_id),
                        CLOSE_CREATE_HISTORICAL_TAB);
      break;
    }

    case CommandRestoreTab: {
      content::RecordAction(UserMetricsAction("TabContextMenu_RestoreTab"));
      delegate_->RestoreTab();
      break;
    }

    case CommandTogglePinned: {
      content::RecordAction(
          UserMetricsAction("TabContextMenu_TogglePinned"));
      std::vector<int> indices = GetIndicesForCommand(context_index);
      bool pin = WillContextMenuPin(context_index);
      if (pin) {
        for (size_t i = 0; i < indices.size(); ++i) {
          if (!IsAppTab(indices[i]))
            SetTabPinned(indices[i], true);
        }
      } else {
        for (size_t i = indices.size(); i > 0; --i) {
          if (!IsAppTab(indices[i - 1]))
            SetTabPinned(indices[i - 1], false);
        }
      }
      break;
    }

    case CommandBookmarkAllTabs: {
      content::RecordAction(
          UserMetricsAction("TabContextMenu_BookmarkAllTabs"));

      delegate_->BookmarkAllTabs();
      break;
    }

    case CommandSelectByDomain:
    case CommandSelectByOpener: {
      std::vector<int> indices;
      if (command_id == CommandSelectByDomain)
        GetIndicesWithSameDomain(context_index, &indices);
      else
        GetIndicesWithSameOpener(context_index, &indices);
      TabStripSelectionModel selection_model;
      selection_model.SetSelectedIndex(context_index);
      for (size_t i = 0; i < indices.size(); ++i)
        selection_model.AddIndexToSelection(indices[i]);
      SetSelectionFromModel(selection_model);
      break;
    }

    default:
      NOTREACHED();
  }
}
