  static base::FilePath install_dir() {
    base::FilePath result;
    PathService::Get(DIR_SW_REPORTER, &result);
    return result;
  }
