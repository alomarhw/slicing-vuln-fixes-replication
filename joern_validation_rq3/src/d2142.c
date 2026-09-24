  void OnCryptohomeNeedsDircryptoMigrationCallback(
      const AccountId& account_id,
      base::Optional<bool> needs_migration) {
    if (!needs_migration.has_value()) {
      LOG(ERROR) << "Failed to call cryptohome NeedsDircryptoMigration.";
      UpdateUI(account_id, false);
      return;
    }

    needs_dircrypto_migration_cache_[account_id] = needs_migration.value();
    UpdateUI(account_id, needs_migration.value());
  }
