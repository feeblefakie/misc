package guice;

import com.google.inject.Inject;

public class DefaultExecutor implements Executor {
  private final Database database;

  @Inject
  public DefaultExecutor(Database database) {
    this.database = database;
  }

  @Override
  public void execute() {
    System.out.println("execute with " + database);
  }
}
