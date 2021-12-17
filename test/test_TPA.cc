//
// Created by Martin Blicha on 06.12.21.
//

#include <gtest/gtest.h>
#include "engine/TPA.h"
#include "Validator.h"

TEST(TPA_test, test_TPA_simple_safe)
{
    LIALogic logic;
    Options options;
    options.addOption(Options::LOGIC, "QF_LIA");
    options.addOption(Options::COMPUTE_WITNESS, "true");
    options.addOption(Options::ENGINE, "tpa-split");
    SymRef s1 = logic.declareFun("s1", logic.getSort_bool(), {logic.getSort_num()}, nullptr, false);
    PTRef x = logic.mkNumVar("x");
    PTRef xp = logic.mkNumVar("xp");
    PTRef current = logic.mkUninterpFun(s1, {x});
    PTRef next = logic.mkUninterpFun(s1, {xp});
    ChcSystem system;
    system.addUninterpretedPredicate(s1);
    system.addClause(
            ChcHead{UninterpretedPredicate{next}},
            ChcBody{logic.mkEq(xp, logic.getTerm_NumZero()), {}});
    system.addClause(
            ChcHead{UninterpretedPredicate{next}},
            ChcBody{logic.mkEq(xp, logic.mkNumPlus(x, logic.getTerm_NumOne())), {UninterpretedPredicate{current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{logic.getTerm_false()}},
            ChcBody{logic.mkNumLt(x, logic.getTerm_NumZero()), {UninterpretedPredicate{current}}}
    );
    auto hypergraph = ChcGraphBuilder(logic).buildGraph(Normalizer(logic).normalize(system));
    ASSERT_TRUE(hypergraph->isNormalGraph());
    auto graph = hypergraph->toNormalGraph();
    TPAEngine engine(logic, options);
    auto res = engine.solve(*graph);
    auto answer = res.getAnswer();
    ASSERT_EQ(answer, VerificationResult::SAFE);
    auto witness = res.getValidityWitness();
    ChcGraphContext ctx(*graph, logic);
    SystemVerificationResult systemResult (std::move(res), ctx);
    auto validationResult = Validator(logic).validate(system, systemResult);
    ASSERT_EQ(validationResult, Validator::Result::VALIDATED);
}

TEST(TPA_test, test_TPA_simple_unsafe)
{
    LIALogic logic;
    Options options;
    options.addOption(Options::LOGIC, "QF_LIA");
    options.addOption(Options::COMPUTE_WITNESS, "true");
    options.addOption(Options::ENGINE, "tpa-split");
    SymRef s1 = logic.declareFun("s1", logic.getSort_bool(), {logic.getSort_num()}, nullptr, false);
    PTRef x = logic.mkNumVar("x");
    PTRef xp = logic.mkNumVar("xp");
    PTRef current = logic.mkUninterpFun(s1, {x});
    PTRef next = logic.mkUninterpFun(s1, {xp});
    ChcSystem system;
    system.addUninterpretedPredicate(s1);
    system.addClause(
            ChcHead{UninterpretedPredicate{next}},
            ChcBody{logic.mkEq(xp, logic.getTerm_NumZero()), {}});
    system.addClause(
            ChcHead{UninterpretedPredicate{next}},
            ChcBody{logic.mkEq(xp, logic.mkNumPlus(x, logic.getTerm_NumOne())), {UninterpretedPredicate{current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{logic.getTerm_false()}},
            ChcBody{logic.mkNumGt(x, logic.getTerm_NumOne()), {UninterpretedPredicate{current}}}
    );
    auto normalizedSystem = Normalizer(logic).normalize(system);
    auto hypergraph = ChcGraphBuilder(logic).buildGraph(normalizedSystem);
    ASSERT_TRUE(hypergraph->isNormalGraph());
    auto graph = hypergraph->toNormalGraph();
    TPAEngine engine(logic, options);
    auto res = engine.solve(*graph);
    auto answer = res.getAnswer();
    ASSERT_EQ(answer, VerificationResult::UNSAFE);
//    auto witness = res.getInvalidityWitness();
    ChcGraphContext ctx(*graph, logic);
    SystemVerificationResult systemResult (std::move(res), ctx);
    auto validationResult = Validator(logic).validate(*normalizedSystem.normalizedSystem, systemResult);
    ASSERT_EQ(validationResult, Validator::Result::VALIDATED);
}

TEST(TPA_test, test_TPA_chain_of_two_unsafe) {
    LIALogic logic;
    Options options;
    options.addOption(Options::LOGIC, "QF_LIA");
    options.addOption(Options::COMPUTE_WITNESS, "true");
    options.addOption(Options::ENGINE, "tpa-split");
    SymRef s1 = logic.declareFun("s1", logic.getSort_bool(), {logic.getSort_num()}, nullptr, false);
    SymRef s2 = logic.declareFun("s2", logic.getSort_bool(), {logic.getSort_num()}, nullptr, false);
    PTRef x = logic.mkNumVar("x");
    PTRef xp = logic.mkNumVar("xp");
    PTRef predS1Current = logic.mkUninterpFun(s1, {x});
    PTRef predS1Next = logic.mkUninterpFun(s1, {xp});
    PTRef predS2Current = logic.mkUninterpFun(s2, {x});
    PTRef predS2Next = logic.mkUninterpFun(s2, {xp});
    ChcSystem system;
    system.addUninterpretedPredicate(s1);
    system.addUninterpretedPredicate(s2);
    system.addClause(
            ChcHead{UninterpretedPredicate{predS1Next}},
            ChcBody{logic.mkEq(xp, logic.getTerm_NumZero()), {}});
    system.addClause(
            ChcHead{UninterpretedPredicate{predS1Next}},
            ChcBody{logic.mkEq(xp, logic.mkNumPlus(x, logic.getTerm_NumOne())), {UninterpretedPredicate{predS1Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{predS2Current}},
            ChcBody{logic.getTerm_true(), {UninterpretedPredicate{predS1Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{predS2Next}},
            ChcBody{logic.mkEq(xp, logic.mkNumMinus(x, logic.getTerm_NumOne())),
                    {UninterpretedPredicate{predS2Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{logic.getTerm_false()}},
            ChcBody{logic.mkNumLt(x, logic.getTerm_NumZero()), {UninterpretedPredicate{predS2Current}}}
    );
    auto hypergraph = ChcGraphBuilder(logic).buildGraph(Normalizer(logic).normalize(system));
    ASSERT_TRUE(hypergraph->isNormalGraph());
    auto graph = hypergraph->toNormalGraph();
    TPAEngine engine(logic, options);
    auto res = engine.solve(*graph);
    ASSERT_EQ(res.getAnswer(), VerificationResult::UNSAFE);
}

TEST(TPA_test, test_TPA_chain_of_two_safe) {
    LIALogic logic;
    Options options;
    options.addOption(Options::LOGIC, "QF_LIA");
    options.addOption(Options::COMPUTE_WITNESS, "true");
    options.addOption(Options::ENGINE, "tpa-split");
    SymRef s1 = logic.declareFun("s1", logic.getSort_bool(), {logic.getSort_num()}, nullptr, false);
    SymRef s2 = logic.declareFun("s2", logic.getSort_bool(), {logic.getSort_num()}, nullptr, false);
    PTRef x = logic.mkNumVar("x");
    PTRef xp = logic.mkNumVar("xp");
    PTRef predS1Current = logic.mkUninterpFun(s1, {x});
    PTRef predS1Next = logic.mkUninterpFun(s1, {xp});
    PTRef predS2Current = logic.mkUninterpFun(s2, {x});
    PTRef predS2Next = logic.mkUninterpFun(s2, {xp});
    ChcSystem system;
    system.addUninterpretedPredicate(s1);
    system.addUninterpretedPredicate(s2);
    system.addClause(
            ChcHead{UninterpretedPredicate{predS1Next}},
            ChcBody{logic.mkEq(xp, logic.getTerm_NumZero()), {}});
    system.addClause(
            ChcHead{UninterpretedPredicate{predS1Next}},
            ChcBody{logic.mkEq(xp, logic.mkNumPlus(x, logic.getTerm_NumOne())), {UninterpretedPredicate{predS1Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{predS2Current}},
            ChcBody{logic.getTerm_true(), {UninterpretedPredicate{predS1Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{predS2Next}},
            ChcBody{logic.mkEq(xp, logic.mkNumPlus(x, logic.mkConst(FastRational(2)))),
                    {UninterpretedPredicate{predS2Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{logic.getTerm_false()}},
            ChcBody{logic.mkNumLt(x, logic.getTerm_NumZero()), {UninterpretedPredicate{predS2Current}}}
    );
    auto hypergraph = ChcGraphBuilder(logic).buildGraph(Normalizer(logic).normalize(system));
    ASSERT_TRUE(hypergraph->isNormalGraph());
    auto graph = hypergraph->toNormalGraph();
    TPAEngine engine(logic, options);
    auto res = engine.solve(*graph);
    ASSERT_EQ(res.getAnswer(), VerificationResult::SAFE);
}

TEST(TPA_test, test_TPA_chain_regression) {
    LIALogic logic;
    Options options;
    options.addOption(Options::LOGIC, "QF_LIA");
//    options.addOption(Options::COMPUTE_WITNESS, "true");
    options.addOption(Options::ENGINE, "tpa-split");
    SymRef s1 = logic.declareFun("inv1", logic.getSort_bool(), {logic.getSort_num(), logic.getSort_num()}, nullptr, false);
    SymRef s2 = logic.declareFun("inv2", logic.getSort_bool(), {logic.getSort_num(), logic.getSort_num()}, nullptr, false);
    PTRef x = logic.mkNumVar("x");
    PTRef xp = logic.mkNumVar("xp");
    PTRef y = logic.mkNumVar("y");
    PTRef yp = logic.mkNumVar("yp");
    PTRef predS1Current = logic.mkUninterpFun(s1, {x, y});
    PTRef predS1Next = logic.mkUninterpFun(s1, {xp, yp});
    PTRef predS2Current = logic.mkUninterpFun(s2, {x, y});
    PTRef predS2Next = logic.mkUninterpFun(s2, {xp, yp});
    PTRef val = logic.mkConst(FastRational(5));
    PTRef doubleVal = logic.mkConst(FastRational(10));
    ChcSystem system;
    system.addUninterpretedPredicate(s1);
    system.addUninterpretedPredicate(s2);
    system.addClause(
            ChcHead{UninterpretedPredicate{predS1Next}},
            ChcBody{logic.mkAnd(logic.mkEq(xp, logic.getTerm_NumZero()), logic.mkEq(yp, val)), {}});
    system.addClause(
            ChcHead{UninterpretedPredicate{predS1Next}},
            ChcBody{logic.mkAnd({
                logic.mkEq(xp, logic.mkNumPlus(x, logic.getTerm_NumOne())),
                logic.mkEq(yp, y),
                logic.mkNumLt(x, val)
                }),
                {UninterpretedPredicate{predS1Current}}
            }
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{predS2Current}},
            ChcBody{logic.mkNumGeq(x, val), {UninterpretedPredicate{predS1Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{predS2Next}},
            ChcBody{logic.mkAnd(
                        logic.mkEq(xp, logic.mkNumPlus(x, logic.getTerm_NumOne())),
                        logic.mkEq(yp, logic.mkNumPlus(y, logic.getTerm_NumOne()))
                        ),
                    {UninterpretedPredicate{predS2Current}}}
    );
    system.addClause(
            ChcHead{UninterpretedPredicate{logic.getTerm_false()}},
            ChcBody{logic.mkAnd(logic.mkEq(x, doubleVal), logic.mkNot(logic.mkEq(x, y))), {UninterpretedPredicate{predS2Current}}}
    );
    auto hypergraph = ChcGraphBuilder(logic).buildGraph(Normalizer(logic).normalize(system));
    ASSERT_TRUE(hypergraph->isNormalGraph());
    auto graph = hypergraph->toNormalGraph();
    TPAEngine engine(logic, options);
    auto res = engine.solve(*graph);
    ASSERT_EQ(res.getAnswer(), VerificationResult::SAFE);
}