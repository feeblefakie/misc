import java.util.List;
import java.util.ArrayList;

public class ListRemoveInFor {
    public static void main(String[] args) {
        List<List<String>> outers = new ArrayList<List<String>>();
        List<String> outer = new ArrayList<String>();
        outer.add("aaaaaaa");
        outer.add("bbbbbbb");
        outer.add("cccccc");
        outers.add(outer);
        List<String> outer2 = new ArrayList<String>();
        outer2.add("aaaaaaa");
        outer2.add("bbbbbbb");
        outer2.add("cccccc");
        outers.add(outer2);

        int i = 0;
        for (List<String> o : outers) {
            List<String> newOuter = new ArrayList<String>();
            newOuter.add("hiroyuki");
            outers.set(i++, newOuter);
        }

        for (List<String> o : outers) {
            System.out.println(o);
        }

    }
}
