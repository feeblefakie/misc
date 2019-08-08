import com.palantir.giraffe.command.Command;
import com.palantir.giraffe.command.CommandResult;
import com.palantir.giraffe.command.Commands;
import com.palantir.giraffe.host.Host;
import com.palantir.giraffe.host.HostControlSystem;
import com.palantir.giraffe.ssh.PublicKeySshCredential;
import com.palantir.giraffe.ssh.SshCredential;
import com.palantir.giraffe.ssh.SshHostAccessor;

import java.io.IOException;
import java.nio.file.Paths;

public class SshTest {
  public static void main(String[] args) {
    SshCredential credential = null;
    try {
      credential =
          PublicKeySshCredential.fromFile("hiroyuki", Paths.get("/Users/hiroyuki/.ssh/id_rsa_backup"));
    } catch (IOException e) {
      e.printStackTrace();
    }
    SshHostAccessor ssh =
        SshHostAccessor.forCredential(Host.fromHostname("localhost"), credential);
    //ssh.request().set("StrictHostKeyChecking", "no");

    HostControlSystem hcs = null;
    try {
      hcs = ssh.open();

      Command remoteCommand =
          hcs.getExecutionSystem()
              .getCommandBuilder("ls")
              .build();

      CommandResult result = Commands.execute(remoteCommand);
      System.out.println(result.getStdOut());
      hcs.close();
    } catch (IOException e) {
      try {
        if (hcs != null) {
          hcs.close();
        }
      } catch (IOException e1) {
        e1.printStackTrace();
      }
    }
  }
}
