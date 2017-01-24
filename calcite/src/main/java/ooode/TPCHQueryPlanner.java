package ooode;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import org.apache.calcite.adapter.enumerable.EnumerableConvention;
import org.apache.calcite.adapter.enumerable.EnumerableRules;
import org.apache.calcite.adapter.java.ReflectiveSchema;
import org.apache.calcite.config.Lex;
import org.apache.calcite.jdbc.CalciteConnection;
import org.apache.calcite.plan.*;
import org.apache.calcite.rel.RelCollationTraitDef;
import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.externalize.RelJsonWriter;
import org.apache.calcite.rel.rules.FilterJoinRule;
import org.apache.calcite.rel.rules.FilterMergeRule;
import org.apache.calcite.rel.type.RelDataTypeSystem;
import org.apache.calcite.rex.RexNode;
import org.apache.calcite.schema.SchemaPlus;
import org.apache.calcite.sql.SqlNode;
import org.apache.calcite.sql.parser.SqlParseException;
import org.apache.calcite.sql.parser.SqlParser;
import org.apache.calcite.tools.*;


public class TPCHQueryPlanner {
	public static void main(String[] args) throws ClassNotFoundException, SQLException, ValidationException, RelConversionException {
        //String sql = "select l.quantity from tpch.customer c join tpch.index_orders2 o on c.custkey = o.custkey join tpch.lineitem l on o.orderkey = l.orderkey where c.custkey < 20000";
        String sql = null;

        for (int i = 0; i < args.length; ++i) {
            if ("-sql".equals(args[i])) {
                sql = args[++i];
            }
        }
        if (sql == null) {
            System.err.println("TPCHQueryPlanner -sql sql");
            System.exit(1);
        }

        RelNode plan = TPCHQueryPlanner.optimize(sql);
	    //System.out.println(RelOptUtil.toString(plan));

        RelJsonWriter writer = new RelJsonWriter();
        plan.explain(writer);
        System.out.println(writer.asString());

        //printPlan(plan);
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

	public static RelNode optimize(String sql)
            throws ClassNotFoundException, SQLException, ValidationException, RelConversionException {
        // TODO Auto-generated method stub
        Class.forName("org.apache.calcite.jdbc.Driver");
        Connection connection =
                DriverManager.getConnection("jdbc:calcite:");
        CalciteConnection calciteConnection =
                connection.unwrap(CalciteConnection.class);
        SchemaPlus rootSchema = calciteConnection.getRootSchema();
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

        return plan;
    }

    public static class Tpch {
        public final Lineitem[] lineitem = {};
        public final Order[] orders = {};
        public final IndexOrder[] index_orders2 = {};
        public final Customer[] customer = {};
    }

    public static class Lineitem {
        public final int orderkey = 0;
        public final int partkey = 0;
        public final int suppkey = 0;
        public final int linenumber = 0;
        public final int quantity = 0;
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
    }

    public static class Order {
        public final int orderkey = 0;
        public final int custkey = 0;
        public final int orderstatus = 0;
        public final int totalprice = 0;
        public final String orderdate = null;
        public final String orderpriority = null;
        public final String clerk = null;
        public final int shippriority = 0;
        public final String comment = null;
    }

    public static class IndexOrder {
        public final int custkey = 0;
        public final int orderkey = 0;
    }

    public static class Customer {
        public final int custkey = 0;
        public final String name = null;
        public final String address = null;
        public final int nationkey = 0;
        public final String phone = null;
        public final int acctbal = 0;
        public final String mktsegment = null;
        public final String comment = null;
    }
}
