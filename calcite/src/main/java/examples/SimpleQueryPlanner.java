package examples;

import java.io.StringWriter;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import org.apache.calcite.adapter.enumerable.EnumerableConvention;
import org.apache.calcite.adapter.enumerable.EnumerableRules;
import org.apache.calcite.adapter.java.JavaTypeFactory;
import org.apache.calcite.adapter.java.ReflectiveSchema;
import org.apache.calcite.adapter.jdbc.JdbcRules;
import org.apache.calcite.config.Lex;
import org.apache.calcite.jdbc.CalciteConnection;
import org.apache.calcite.jdbc.CalciteSchema;
import org.apache.calcite.jdbc.JavaTypeFactoryImpl;
import org.apache.calcite.linq4j.Enumerable;
import org.apache.calcite.plan.*;
import org.apache.calcite.prepare.CalciteCatalogReader;
import org.apache.calcite.rel.RelCollationTraitDef;
import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.RelWriter;
import org.apache.calcite.rel.externalize.RelJsonWriter;
import org.apache.calcite.rel.rules.FilterJoinRule;
import org.apache.calcite.rel.rules.FilterMergeRule;
import org.apache.calcite.rel.type.RelDataTypeSystem;
import org.apache.calcite.rex.RexNode;
import org.apache.calcite.rex.RexShuttle;
import org.apache.calcite.schema.SchemaPlus;
import org.apache.calcite.sql.SqlNode;
import org.apache.calcite.sql.fun.SqlStdOperatorTable;
import org.apache.calcite.sql.parser.SqlParseException;
import org.apache.calcite.sql.parser.SqlParser;
import org.apache.calcite.sql.validate.SqlConformance;
import org.apache.calcite.sql.validate.SqlValidator;
import org.apache.calcite.sql.validate.SqlValidatorUtil;
import org.apache.calcite.tools.*;


public class SimpleQueryPlanner {
	public static void main(String[] args) throws ClassNotFoundException, SQLException, ValidationException, RelConversionException {
        //String sql = "select * from os.orders o1 join os.orders o2 on o1.orderid = o2.orderid where o1.productid > 10";
        //String sql = "select * from os.orders where productid > 10";
        //String sql = "select l.quantity from tpch.customer c join tpch.orders o on c.custkey = o.custkey join tpch.lineitem l on o.orderkey = l.orderkey where c.custkey < 20000";
        //String sql = "select c.name, l.quantity from tpch.customer c join tpch.index_orders2 o on c.custkey = o.custkey join tpch.lineitem l on o.orderkey = l.orderkey where c.custkey < 20000 and c.nationkey > 1";
        String sql = "select c.name from tpch.customer c where c.custkey < 20000";
        RelNode plan = SimpleQueryPlanner.getOptimizedPlan1(sql);
        //RelNode plan = SimpleQueryPlanner.getOptimizedPlan2(sql);
	    System.out.println(RelOptUtil.toString(plan));


        RelJsonWriter writer = new RelJsonWriter();
        plan.explain(writer);
        System.out.println(writer.asString());

        printPlan(plan);
	}

	public static void printPlan(RelNode root) {
        System.out.println(root);
        for (RexNode rex : root.getChildExps()) {
            System.out.println("==" + rex.getKind());
            System.out.println("==" + rex.getType());
            System.out.println("==" + rex.toString());
        }
        //System.out.println("    " + root.accept(new RexShuttle()));
        for (RelNode node : root.getInputs()) {
            printPlan(node);
        }
    }

	public static RelNode getOptimizedPlan1(String sql) throws ClassNotFoundException, SQLException, ValidationException, RelConversionException {
        // TODO Auto-generated method stub
        Class.forName("org.apache.calcite.jdbc.Driver");
        Connection connection =
                DriverManager.getConnection("jdbc:calcite:");
        CalciteConnection calciteConnection =
                connection.unwrap(CalciteConnection.class);
        SchemaPlus rootSchema = calciteConnection.getRootSchema();
        //rootSchema.add("os", new ReflectiveSchema(new Os()));
        rootSchema.add("tpch", new ReflectiveSchema(new Tpch()));

        final List<RelTraitDef> traitDefs = new ArrayList<RelTraitDef>();
        traitDefs.add(ConventionTraitDef.INSTANCE);
        traitDefs.add(RelCollationTraitDef.INSTANCE);

        Program program =
                Programs.ofRules(
                        FilterMergeRule.INSTANCE,
                        FilterJoinRule.FILTER_ON_JOIN,
                        EnumerableRules.ENUMERABLE_FILTER_RULE,
                        EnumerableRules.ENUMERABLE_JOIN_RULE,
                        EnumerableRules.ENUMERABLE_PROJECT_RULE);

        FrameworkConfig calciteFrameworkConfig = Frameworks.newConfigBuilder()
                .parserConfig(SqlParser.configBuilder()
                        .setLex(Lex.MYSQL)
                        .build())
                .defaultSchema(rootSchema)
                .traitDefs(traitDefs)
                .context(Contexts.EMPTY_CONTEXT)
                .ruleSets(RuleSets.ofList())
                .costFactory(null)
                .programs(program)
                .typeSystem(RelDataTypeSystem.DEFAULT)
                .build();

        Planner planner = Frameworks.getPlanner(calciteFrameworkConfig);

        String query = sql;
        SqlNode sqlNode;
        try {
            sqlNode = planner.parse(query);
        } catch (SqlParseException e) {
            throw new RuntimeException("Query parsing error.", e);
        }
        SqlNode validatedSqlNode = planner.validate(sqlNode);
        RelNode plan = planner.rel(validatedSqlNode).project();
        //RelTraitSet traitSet = planner.getEmptyTraitSet().replace(Convention.NONE);
        RelTraitSet traitSet = planner.getEmptyTraitSet().replace(EnumerableConvention.INSTANCE);
        plan = planner.transform(0, traitSet, plan);

        /*
        RelTraitSet traitSet = plan.getTraitSet();
        traitSet = traitSet.simplify(); // TODO: Is this the correct thing to do? Why relnode has a composite trait?
        plan = planner.transform(0, traitSet.plus(EnumerableConvention.INSTANCE), plan);
        */

        return plan;
    }

    /*
    public static RelNode getOptimizedPlan2(String sql) throws ClassNotFoundException, SQLException, ValidationException, RelConversionException, SqlParseException {
        SqlParser parser = SqlParser.create(sql, SqlParser.Config.DEFAULT);
        SqlNode sqlNode = parser.parseStmt();

        Class.forName("org.apache.calcite.jdbc.Driver");
        Connection connection =
                DriverManager.getConnection("jdbc:calcite:");
        CalciteConnection calciteConnection =
                connection.unwrap(CalciteConnection.class);
        SchemaPlus rootSchema = calciteConnection.getRootSchema();
        rootSchema.add("os", new ReflectiveSchema(new Os()));

        JavaTypeFactory typeFactory = new JavaTypeFactoryImpl(RelDataTypeSystem.DEFAULT);
        CalciteCatalogReader catalogReader = new CalciteCatalogReader(
                CalciteSchema.from(rootSchema),
                SqlParser.Config.DEFAULT.caseSensitive(),
                CalciteSchema.from(rootSchema).path(null),
                typeFactory);


        SqlValidator validator = SqlValidatorUtil.newValidator(SqlStdOperatorTable.instance(), catalogReader, typeFactory, SqlConformance.DEFAULT);
        SqlNode validatedSqlNode = validator.validate(sqlNode);

    }
    */

    /*
	public static class Os {
	    public final Order[] orders = {
	      new Order(3,1,12),
	      new Order(5,2,7),
	      new Order(15, 3,4),
	    };
	}

	public static class Order {
		public final int productid;
	    public final int orderid;
	    public final int units;
	    

	    public Order(int productid,int orderid, int units ) {
	      this.productid=productid;
	      this.orderid=orderid;
	      this.units=units;      
	    }
	}
	*/

    public static class Tpch {
        public final Lineitem[] lineitem = {
                new Lineitem(1,100,100,1,1),
                new Lineitem(2,100,100,1,1),
                new Lineitem(3,100,100,1,1),
        };
        public final Order[] orders = {
                new Order(1,1),
                new Order(2,2),
                new Order(3,2),
        };
        public final IndexOrder[] index_orders2 = {
                new IndexOrder(1,1),
                new IndexOrder(2,2),
                new IndexOrder(2,3),
        };
        public final Customer[] customer = {
                new Customer(1, "customer1", "tokyo", 1),
                new Customer(2, "customer2", "tokyo", 1),
        };
    }

    public static class Lineitem {
        public final int orderkey;
        public final int partkey;
        public final int suppkey;
        public final int linenumber;
        public final int quantity;
        public final int extendedprice = 0;
        public final int discount = 0;
        public final int tax = 0;
        public final boolean returnflag = false;
        public final int linestatus = 0;
        public final String shipdate = null;
        public final String commitdate = null;
        public final String receiptdate = null;
        public final String shipinstruct = null;
        public final String shipmode = null;
        public final String comment = null;

        public Lineitem(int orderkey, int partkey, int suppkey, int linenumber, int quantity) {
            this.orderkey = orderkey;
            this.partkey = partkey;
            this.suppkey = suppkey;
            this.linenumber = linenumber;
            this.quantity = quantity;
        }
    }

    public static class Order {
        public final int orderkey;
        public final int custkey;
        public final int orderstatus = 0;
        public final int totalprice = 0;
        public final String orderdate = null;
        public final String orderpriority = null;
        public final String clerk = null;
        public final int shippriority = 0;
        public final String comment = null;

        public Order(int orderkey, int custkey) {
            this.orderkey = orderkey;
            this.custkey = custkey;
        }
    }

    public static class IndexOrder {
        public final int custkey;
        public final int orderkey;

        public IndexOrder(int custkey, int orderkey) {
            this.custkey = custkey;
            this.orderkey = orderkey;
        }
    }

    public static class Customer {
        public final int custkey;
        public final String name;
        public final String address;
        public final int nationkey;
        public final String phone = null;
        public final int acctbal = 0;
        public final String mktsegment = null;
        public final String comment = null;

        public Customer(int custkey, String name, String address, int nationkey) {
            this.custkey = custkey;
            this.name = name;
            this.address = address;
            this.nationkey = nationkey;
        }
    }
}
