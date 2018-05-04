package guice;

import com.google.inject.Inject;

public class LedgerService {
  private final Manager manager;
  private final Executor executor;
  private final Config config;

  @Inject
  public LedgerService(Manager manager, Executor executor, Config config) {
    this.manager = manager;
    this.executor = executor;
    this.config = config;
  }

  public void doSomething() {
    manager.manage();
    executor.execute();
    config.print();
  }
}
