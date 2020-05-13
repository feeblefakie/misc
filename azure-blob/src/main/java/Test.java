import com.azure.identity.DefaultAzureCredentialBuilder;
import com.azure.storage.blob.BlobAsyncClient;
import com.azure.storage.blob.BlobContainerAsyncClient;
import com.azure.storage.blob.BlobServiceAsyncClient;
import com.azure.storage.blob.BlobServiceClientBuilder;

public class Test {
  public static void main(String[] args) {
    BlobServiceClientBuilder builder = new BlobServiceClientBuilder();
    System.out.println("###" + System.getenv("AZURE_CLIENT_ID") + "###");
    System.out.println("###" + System.getenv("AZURE_TENANT_ID") + "###");
    System.out.println("###" + System.getenv("AZURE_CLIENT_SECRET") + "###");

    // Use service principal or managed identity when a connection string is not given
    String accountUrl = "https://cassydev.blob.core.windows.net";
    builder.endpoint(accountUrl).credential(new DefaultAzureCredentialBuilder().build());
    BlobServiceAsyncClient asyncClient = builder.buildAsyncClient();

    BlobContainerAsyncClient client = asyncClient.getBlobContainerAsyncClient("scalar-test");

    BlobAsyncClient blobClient = client.getBlobAsyncClient("myblockblob");
    blobClient.uploadFromFile("/tmp/file.txt").block();
  }
}
