package db.hadooode;

import org.apache.hadoop.util.StringUtils;

public class TestString {
    public static void main(String[] args) {
        //String str = "aaa|bbb|cccc|dddd|eee";
        String str = "32|85811|8320|5|44|79059.64|0.05|0.06|N|O|1995-08-28|1995-08-20|1995-09-14|DELIVER IN PERSON|AIR|symptotes nag according to the ironic depo|";
        String item = null;
        int offset = 15;

        long start = System.currentTimeMillis();
        for (int i = 0; i < 1000000; ++i) {
            item = getByIndex(str, offset);
        }
        long end = System.currentTimeMillis();
        System.out.println("item: " + item);
        System.out.println("getByIndex time(s): " + (end - start) / 1000.0);

        start = System.currentTimeMillis();
        for (int i = 0; i < 1000000; ++i) {
            String[] items = str.split("\\|");
            item = items[offset];
        }
        end = System.currentTimeMillis();
        System.out.println("item: " + item);
        System.out.println("String.split time(s): " + (end - start) / 1000.0);

        start = System.currentTimeMillis();
        for (int i = 0; i < 1000000; ++i) {
            String[] items = StringUtils.split(str, '\\', '|');
            item = items[offset];
        }
        end = System.currentTimeMillis();
        System.out.println("item: " + item);
        System.out.println("StringUtils.split time(s): " + (end - start) / 1000.0);

    }

    public static String getByIndex(String str, int index) {
        return getByIndex(str, index, '|', '\\');
    }

    public static String getByIndex(String str, int index, char separator, char escapeChar) {
        int i = 0;
        int offset = 0;
        while (i++ < index) {
            offset = str.indexOf(separator, offset);
            offset++;
        }
        StringBuilder builder = new StringBuilder();
        int end = StringUtils.findNext(str, separator, escapeChar, offset, builder);
        return builder.toString();
    }
}
