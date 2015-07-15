import java.util.List;
import java.util.ArrayList;

class ListContainer {
    private final List<String> list;

    public ListContainer() {
        list = new ArrayList<String>();
    }

    public void add(String val) {
        list.add(val);
    }

    public List<String> getAll() {
        return list;
    }

    public String toString() {
        return list.toString();
    }

    public static void main(String[] args) {
        ListContainer list = new ListContainer();
        list.add("hello");
        list.add("world");
        System.out.println("list: " + list);

        List<String> ref = list.getAll();
        ref.set(0, "fuck");
        System.out.println("list: " + list);
   }
}

