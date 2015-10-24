package junit.tutorial;

/**
 * Created by hiroyuki on 10/24/15.
 */
public class Calculator {
    public int multiply(int x, int y) {
        return x * y;
    }
    public float divide(int x, int y) {
        if (y == 0) {
            throw new IllegalArgumentException("dividing by zero");
        }
        return (float) x / (float) y;
    }
}
