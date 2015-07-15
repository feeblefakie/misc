import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import org.apache.lucene.analysis.ja.JapaneseAnalyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.queryParser.QueryParser;
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Searcher;

class Search {
    static final String index = "index";
    public static void main(String[] args) {
        try {
            Searcher searcher = new IndexSearcher(index);
            BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
            if (args.length != 1) {
                System.out.println("key");
                System.exit(-1);
            }
            JapaneseAnalyzer ja = new JapaneseAnalyzer();
            long start = System.currentTimeMillis();
            QueryParser qparser = new QueryParser("contents", ja);
            Query query = qparser.parse(args[0]);
            Hits hits = searcher.search(query);
            long stop = System.currentTimeMillis();
            System.out.println(hits.length() + " hits");
            System.out.println(stop-start + " milliseconds");
            int num_disp = hits.length() > 5 ? 5 : hits.length();
            for (int i = 0; i < num_disp; i++) {
                Document doc = hits.doc(i);
                System.out.println("id："+doc.get("id"));
                System.out.println("title："+doc.get("title"));
                System.out.println("entry："+doc.get("entry"));
            }
            searcher.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
