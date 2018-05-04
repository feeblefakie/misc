package guice;

public class Config {
  private final String name;

  public Config(String name) {
    this.name = name;
  }

  public void print() {
    System.out.println(name);
  }
}
