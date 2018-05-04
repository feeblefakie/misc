package guice;

import com.google.inject.Guice;
import com.google.inject.Injector;
import guice.module.LedgerModule;

public class Main {
  public static void main(String[] args) {
    Injector injector = Guice.createInjector(new LedgerModule());
    LedgerService service = injector.getInstance(LedgerService.class);

    service.doSomething();
  }
}
