package cosmosdb;

public class AccountWithBalance {
  private String id;
  private Integer balance;

  public AccountWithBalance() {}

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public Integer getBalance() {
    return balance;
  }

  public void setBalance(Integer balance) {
    this.balance = balance;
  }

  @Override
  public String toString() {
    return id + ":" + balance;
  }
}
