import java.io.*;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.Attr;
import javax.xml.parsers.*;
import java.util.HashMap;

public class SimpleDOMParser {
    public static void main(String args[]) throws Exception{
        HashMap<String, Element> replicaMap = new HashMap<String, Element>();
        Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(new File("replica.xml"));
        Element root = document.getDocumentElement();
        NodeList replicaNodes = root.getElementsByTagName("replica");
        for(int i = 0; i < replicaNodes.getLength(); i++) {
            Element replica = (Element) replicaNodes.item(i);
            replicaMap.put(replica.getAttribute("name"), replica);
            /*
            System.out.println(replica.getAttribute("name"));
            NodeList nodes = replica.getChildNodes();
            for (int j = 0; j < nodes.getLength(); j++) {
                if (nodes.item(j).getNodeName().equals("type")) {
                    Element replicaElement = (Element) nodes.item(j);
                    System.out.println(replicaElement.getFirstChild().getNodeValue());
                }
            }
            */
        }

        Element replica = replicaMap.get("lineitem.primary.index");
        if (replica != null) {
            System.out.println(replica.getAttribute("name"));
            NodeList nodes = replica.getChildNodes();
            for (int j = 0; j < nodes.getLength(); j++) {
                if (nodes.item(j).getNodeName().equals("type")) {
                    Element replicaElement = (Element) nodes.item(j);
                    System.out.println(replicaElement.getFirstChild().getNodeValue());
                }
                if (nodes.item(j).getNodeName().equals("key")) {
                    Element replicaElement = (Element) nodes.item(j);
                    System.out.println(replicaElement.getFirstChild().getNodeValue());
                }
                if (nodes.item(j).getNodeName().equals("base")) {
                    Element replicaElement = (Element) nodes.item(j);
                    System.out.println(replicaElement.getFirstChild().getNodeValue());
                }
            }
        }
    }
}
