package guice.module;

import com.google.inject.AbstractModule;
import com.google.inject.Provides;
import guice.Config;
import guice.Database;
import guice.DefaultDatabase;
import guice.DefaultExecutor;
import guice.DefaultManager;
import guice.Executor;
import guice.Manager;

public class LedgerModule extends AbstractModule {

  @Override
  protected void configure() {
    bind(Executor.class).to(DefaultExecutor.class);
    bind(Database.class).to(DefaultDatabase.class);
    bind(Manager.class).to(DefaultManager.class);
  }

  @Provides
  Config provideConfig() {
    return new Config("OHHHHH");
  }
}
