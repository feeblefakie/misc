package junit.tutorial;

//import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import static org.hamcrest.CoreMatchers.*;

/**
 * Created by hiroyuki on 10/24/15.
 */
public class CalculatorTest {

    /*
    @Before
    public void setUp() throws Exception {

    }
    */

    @Test
    public void testMultiplyThreeByFour() {
        Calculator calc = new Calculator();
        int expected = 12;
        int actual = calc.multiply(3, 4);
        assertThat(actual, is(expected));
    }
    @Test
    public void testMultiplyFiveByTwo() {
        Calculator calc = new Calculator();
        int expected = 10;
        int actual = calc.multiply(5, 2);
        assertThat(actual, is(expected));
    }
    @Test
    public void testDivideThreeByFive() {
        Calculator calc = new Calculator();
        float expected = 0.6f;
        float actual = calc.divide(3, 5);
        assertThat(actual, is(expected));
    }
    @Test(expected = IllegalArgumentException.class)
    public void testDivideFiveByZero() {
        Calculator calc = new Calculator();
        calc.divide(5, 0);
    }
}