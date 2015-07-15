import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.document.Fieldable;
//import org.apache.lucene.analysis.cjk.CJKAnalyzer;
import org.apache.lucene.analysis.ja.JapaneseAnalyzer;
import java.io.*;

public class Indexer {

    static final File INDEX_DIR = new File("index");

    public static void main(String[] args) {
        try {
            FileInputStream fis = new FileInputStream("data.txt");
            InputStreamReader ir = new InputStreamReader(fis);
            BufferedReader br = new BufferedReader(ir);

            IndexWriter writer = new IndexWriter(INDEX_DIR, new JapaneseAnalyzer(), true);

            int i = 0;
            String line;
            while ((line = br.readLine()) != null) {
                ++i;
                String[] items = line.split("");
                if (items.length != 3) {
                    continue;
                }

                Document doc = new Document();

                doc.add(new Field("id", items[0], Field.Store.YES, Field.Index.NO));
                doc.add(new Field("title", items[1], Field.Store.YES, Field.Index.NO));
                doc.add(new Field("entry", items[2], Field.Store.COMPRESS, Field.Index.NO));
                doc.add(new Field("contents", items[1] + " " + items[2], Field.Store.NO, Field.Index.TOKENIZED));

                writer.addDocument(doc);
                System.out.println(i + ": " + items[0] + " indexed.");
                
            }

            System.out.println("Optimizing...");
            writer.optimize();
            writer.close();

            fis.close();
            ir.close();
            fis.close();
        } catch (Exception e) {
            System.out.println(e);
        }
    }
}
