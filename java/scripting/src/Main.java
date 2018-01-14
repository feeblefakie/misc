import jdk.nashorn.api.scripting.ScriptObjectMirror;

import javax.script.Invocable;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import java.util.HashMap;
import java.util.Map;

public class Main {
    private static final String SAMPLE_JSON = "{\"value\":-20,\"string\":\"hogehoge\",\"map\":{\"key2\":\"value2\",\"key1\":\"value1\"},\"list\":[\"item1\",\"item2\"]}";

    public static void main(String[] arguments) {
        ScriptEngineManager manager = new ScriptEngineManager();
        ScriptEngine engine = manager.getEngineByName("javascript");
        Map<String, String> x = new HashMap<>();
        x.put("balance", "10");
        Map<String, String> y = new HashMap<>();
        y.put("balance", "100");
        Map[] accounts = {x, y};

        try {
            for (int i = 0; i < 10; i++) {
                long start = System.currentTimeMillis();
                ScriptObjectMirror json = (ScriptObjectMirror) engine.eval("JSON");

                String script = "function echo(message) { print(message);" +
                        "print(json.value); " +
                        "print(accounts[0].balance); print(accounts[1].balance); var result = getdata(); " +
                        //"return JSON.stringify(result); " +
                        "json.value += 1000;" +
                        "return json;" +
                        "} ";
                script += "function getdata() { var data = JSON.parse(sample); print(data.value + 100); print(data.list[1]); data.value += 100; return data; }";
                engine.put("accounts", accounts);
                engine.put("sample", SAMPLE_JSON);
                engine.put("json", json.callMember("parse", SAMPLE_JSON));
                engine.eval(script);

                Invocable invocable = (Invocable) engine;
                Object result = invocable.invokeFunction("echo", "hello");
                //String resultString = (String) json.callMember("stringify", result);
                //System.out.println(resultString);
                long end = System.currentTimeMillis();
                System.out.println("### " + (end - start));
            }
        } catch (ScriptException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }
}
