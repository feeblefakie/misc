package org.apache.hadoop.hive.ql.io;

public class PredicateInfo {
    private String start;
    private String end;
    private boolean isStartContainsBound;
    private boolean isEndContainsBound;
    private String op;
    private String column;
    private String type;
    private String value;

    // "<(p_retailprice,1000)"
    // TODO: handle range predicate
    public PredicateInfo(String predicate) {
        // null means the beginning and the end of the full range
        start = null;
        end = null;
        int from = 0;
        int to = predicate.indexOf("(");
        op = predicate.substring(from, to);
        from = to;
        to = predicate.indexOf(":", from+1);
        column = predicate.substring(from+1, to);
        from = to;
        to = predicate.indexOf(",", from+1);
        type = predicate.substring(from+1, to);
        from = to;
        to = predicate.indexOf(")", from+1);
        value = predicate.substring(from+1, to);
        
        if (op.equals("=")) {
            isStartContainsBound = true;
            isEndContainsBound = true;
            start = value;
            end = value;
        } else if (op.equals("<")) {
            isStartContainsBound = true;
            isEndContainsBound = false;
            end = value;
        } else if (op.equals("<=")) {
            isStartContainsBound = true;
            isEndContainsBound = true;
            end = value;
        } else if (op.equals(">")) {
            isStartContainsBound = false;
            isEndContainsBound = true;
            start = value;
        } else if (op.equals(">=")) {
            isStartContainsBound = true;
            isEndContainsBound = true;
            start = value;
        }
    }

    public String getStart() {
        return start;
    }
    public String getEnd() {
        return end;
    }
    public boolean isStartContainsBound() {
        return isStartContainsBound;
    }
    public boolean isEndContainsBound() {
        return isEndContainsBound;
    }
    public String getOp() {
        return op;
    }
    public String getColumn() {
        return column;
    }
    public String getType() {
        return type;
    }
    public String getValue() {
        return value;
    }
}
