package guice;

import com.google.inject.Inject;

public class LedgerService {
  private final Manager manager;
  private final Executor executor;
  private final Config config;
  private final Worker worker;

  @Inject
  public LedgerService(Manager manager, Executor executor, Config config, Worker worker) {
    this.manager = manager;
    this.executor = executor;
    this.config = config;
    this.worker = worker;
  }

  public void doSomething() {
    manager.manage();
    executor.execute();
    config.print();
    worker.doWork();
  }
}
