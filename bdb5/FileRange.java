package org.apache.hadoop.hive.ql.io;

import java.io.Serializable;

public class FileRange implements Serializable {
    private long offset;
    private int length;
    
    public FileRange(long offset, int length) {
        this.offset = offset;
        this.length = length;
    }
    public void setOffset(long offset) {
        this.offset = offset;
    }
    public long getOffset() {
        return offset;
    }
    public void setLength(int length) {
        this.length = length;
    }
    public int getLength() {
        return length;
    }
}
