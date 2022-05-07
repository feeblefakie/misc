package cosmosdb;

import com.azure.cosmos.ConsistencyLevel;
import com.azure.cosmos.CosmosClient;
import com.azure.cosmos.CosmosClientBuilder;
import com.azure.cosmos.CosmosContainer;
import com.azure.cosmos.CosmosException;
import com.azure.cosmos.models.CosmosItemRequestOptions;
import com.azure.cosmos.models.CosmosItemResponse;
import com.azure.cosmos.models.CosmosStoredProcedureRequestOptions;
import com.azure.cosmos.models.CosmosStoredProcedureResponse;
import com.azure.cosmos.models.PartitionKey;
import java.util.Arrays;

public class Test {
  public static void main(String[] args) {
    String host = System.getenv("ACCOUNT_HOST");
    String key = System.getenv("ACCOUNT_KEY");

    System.out.println(host);
    System.out.println(key);

    CosmosClient client =
        new CosmosClientBuilder()
            .endpoint(host)
            .key(key)
            .directMode()
            .consistencyLevel(ConsistencyLevel.STRONG)
            .buildClient();

    CosmosContainer container = client.getDatabase("banking").getContainer("account");
    create(container);
    read(container);
    upsert(container);
    read(container);
    // replace(container);
    // upsert2(container);
    upsertSp(container);
    read(container);

    client.close();
  }

  private static void create(CosmosContainer container) {
    Account account = new Account();
    account.setId("xxx");
    account.setBalance(1000);
    account.setCreatedAt(System.currentTimeMillis());
    account.setUpdatedAt(System.currentTimeMillis());
    CosmosItemResponse resp = null;
    try {
      resp = container.createItem(
        account, new PartitionKey(account.getId()), new CosmosItemRequestOptions());
    } catch (CosmosException e) {
      e.printStackTrace();
      System.out.println(e.getStatusCode());
    }
  }

  private static void read(CosmosContainer container) {
    CosmosItemResponse<Account> resp =
        container.readItem("xxx", new PartitionKey("xxx"), Account.class);
    System.out.println("etag: " + resp.getETag());
    System.out.println("item: " + resp.getItem());
  }

  private static void upsert(CosmosContainer container) {
    Account account = new Account();
    account.setId("xxx");
    account.setBalance(900);
    account.setCreatedAt(System.currentTimeMillis());
    account.setUpdatedAt(System.currentTimeMillis());

    CosmosItemResponse<Account> resp =
        container.upsertItem(account, new PartitionKey("xxx"), new CosmosItemRequestOptions());
    System.out.println("etag: " + resp.getETag());
    System.out.println("item: " + resp.getItem());
  }

  private static void upsert2(CosmosContainer container) {
    // NOTE: This will delete createdAt and updatedAt column
    AccountWithBalance account = new AccountWithBalance();
    account.setId("xxx");
    account.setBalance(10000);

    CosmosItemResponse<AccountWithBalance> resp =
        container.upsertItem(account, new PartitionKey("xxx"), new CosmosItemRequestOptions());
    System.out.println("etag: " + resp.getETag());
    System.out.println("item: " + resp.getItem());
  }

  private static void replace(CosmosContainer container) {
    Account account = new Account();
    account.setId("xxx");
    account.setBalance(800);

    CosmosItemResponse<Account> resp =
        container.replaceItem(
            account, "xxx", new PartitionKey("xxx"), new CosmosItemRequestOptions());
  }

  private static void upsertSp(CosmosContainer container) {
    // NOTE: This will NOT delete createdAt and updatedAt column and keep them as they are, thus,
    // it's more like an upsert operation
    AccountWithBalance account = new AccountWithBalance();
    account.setId("xxx");
    account.setBalance(100000);

    CosmosStoredProcedureResponse response =
        container
            .getScripts()
            .getStoredProcedure("test")
            .execute(
                Arrays.asList(account),
                new CosmosStoredProcedureRequestOptions()
                    .setPartitionKey(new PartitionKey(account.getId())));

    System.out.println(response.getScriptLog());
    System.out.println(response.getResponseAsString());
  }
}
