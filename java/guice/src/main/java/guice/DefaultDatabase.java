package guice;

public class DefaultDatabase implements Database {

  @Override
  public void query(String query) {
    System.out.println("query with " + query);
  }
}
