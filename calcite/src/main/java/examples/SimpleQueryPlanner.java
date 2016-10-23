package examples;

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
import org.apache.calcite.rel.rules.FilterJoinRule;
import org.apache.calcite.rel.rules.FilterMergeRule;
import org.apache.calcite.rel.type.RelDataTypeSystem;
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
        String sql = "select * from os.orders o1 join os.orders o2 on o1.orderid = o2.orderid where o1.productid > 10";
        //String sql = "select * from os.orders where productid > 10";
        RelNode plan = SimpleQueryPlanner.getOptimizedPlan1(sql);
        //RelNode plan = SimpleQueryPlanner.getOptimizedPlan2(sql);
	    System.out.println(RelOptUtil.toString(plan));
	}

	public static RelNode getOptimizedPlan1(String sql) throws ClassNotFoundException, SQLException, ValidationException, RelConversionException {
        // TODO Auto-generated method stub
        Class.forName("org.apache.calcite.jdbc.Driver");
        Connection connection =
                DriverManager.getConnection("jdbc:calcite:");
        CalciteConnection calciteConnection =
                connection.unwrap(CalciteConnection.class);
        SchemaPlus rootSchema = calciteConnection.getRootSchema();
        rootSchema.add("os", new ReflectiveSchema(new Os()));

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
}
