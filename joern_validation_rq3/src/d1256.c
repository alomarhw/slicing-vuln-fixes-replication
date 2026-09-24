void MetricsWebContentsObserver::RemoveTestingObserver(
    TestingObserver* observer) {
  testing_observers_.RemoveObserver(observer);
}
