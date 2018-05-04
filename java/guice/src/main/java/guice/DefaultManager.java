package guice;

public class DefaultManager implements Manager {

  @Override
  public void manage() {
    System.out.println("managing");
  }
}
