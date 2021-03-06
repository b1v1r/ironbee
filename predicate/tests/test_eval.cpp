/*****************************************************************************
 * Licensed to Qualys, Inc. (QUALYS) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * QUALYS licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

/**
 * @file
 * @brief Predicate --- Eval Tests
 *
 * @author Christopher Alfeld <calfeld@qualys.com>
 **/

#include <ironbee/predicate/eval.hpp>
#include <ironbee/predicate/value.hpp>
#include <ironbeepp/test_fixture.hpp>

#include "gtest/gtest.h"

#ifdef __clang__
#pragma clang diagnostic push
#if __has_warning("-Wunused-local-typedef")
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif
#include <boost/lexical_cast.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

using namespace IronBee::Predicate;
using namespace std;

class TestEval : public ::testing::Test, public IronBee::TestFixture
{
};

TEST_F(TestEval, NodeEvalState_Trivial)
{
    NodeEvalState nes;

    EXPECT_FALSE(nes.is_finished());
    EXPECT_FALSE(nes.is_forwarding());
    EXPECT_FALSE(nes.is_aliased());
    EXPECT_FALSE(nes.forwarded_to());
    EXPECT_EQ(IB_PHASE_NONE, nes.phase());
    EXPECT_FALSE(nes.value());
    EXPECT_TRUE(nes.state().empty());
}

TEST_F(TestEval, NodeEvalState_Finish)
{
    {
        NodeEvalState nes;

        EXPECT_FALSE(nes.is_finished());
        EXPECT_NO_THROW(nes.finish());
        EXPECT_TRUE(nes.is_finished());
        EXPECT_THROW(nes.finish(), IronBee::einval);
    }

    {
        NodeEvalState nes;

        EXPECT_FALSE(nes.is_finished());
        EXPECT_NO_THROW(nes.finish());
        EXPECT_TRUE(nes.is_finished());
        EXPECT_THROW(nes.finish(), IronBee::einval);
        EXPECT_FALSE(nes.value());
    }

    {
        NodeEvalState nes;

        EXPECT_FALSE(nes.is_finished());
        EXPECT_NO_THROW(nes.finish_true(m_transaction));
        EXPECT_TRUE(nes.is_finished());
        EXPECT_THROW(nes.finish(), IronBee::einval);
        EXPECT_TRUE(nes.value());
    }
}

TEST_F(TestEval, NodeEvalState_Local)
{
    NodeEvalState nes;

    nes.setup_local_list(m_transaction.memory_manager());
    ASSERT_FALSE(nes.value());
    EXPECT_TRUE(nes.value().as_list().empty());
    EXPECT_FALSE(nes.is_forwarding());
    EXPECT_FALSE(nes.is_aliased());
    EXPECT_FALSE(nes.forwarded_to());

    nes.append_to_list(Value());
    EXPECT_EQ(1UL, nes.value().as_list().size());

    EXPECT_THROW(nes.forward(NULL), IronBee::einval);
    EXPECT_THROW(nes.alias(Value()), IronBee::einval);
    EXPECT_NO_THROW(nes.setup_local_list(m_transaction.memory_manager()));

    EXPECT_NO_THROW(nes.finish());
    EXPECT_TRUE(nes.is_finished());
}

TEST_F(TestEval, NodeEvalState_Forwarded)
{
    node_p n(new Literal());

    NodeEvalState nes;

    nes.forward(n.get());
    EXPECT_TRUE(nes.is_forwarding());
    EXPECT_EQ(n.get(), nes.forwarded_to());

    EXPECT_THROW(nes.setup_local_list(m_transaction.memory_manager()), IronBee::einval);
    EXPECT_THROW(nes.forward(NULL), IronBee::einval);
    EXPECT_THROW(nes.alias(Value()), IronBee::einval);
    EXPECT_THROW(nes.finish(), IronBee::einval);
    EXPECT_THROW(nes.append_to_list(Value()), IronBee::einval);
}

TEST_F(TestEval, NodeEvalState_Aliased)
{
    IronBee::ScopedMemoryPoolLite mpl;
    IronBee::Field f = IronBee::Field::create_number(mpl, "", 0, 5);
    Value v(f);
    NodeEvalState nes;

    nes.alias(v);
    EXPECT_TRUE(nes.is_aliased());
    EXPECT_EQ(v, nes.value());

    EXPECT_THROW(nes.setup_local_list(m_transaction.memory_manager()), IronBee::einval);
    EXPECT_THROW(nes.forward(NULL), IronBee::einval);
    EXPECT_THROW(nes.alias(Value()), IronBee::einval);
    EXPECT_THROW(nes.append_to_list(Value()), IronBee::einval);

    EXPECT_NO_THROW(nes.finish());
    EXPECT_TRUE(nes.is_finished());
}

TEST_F(TestEval, NodeEvalState_Phase)
{
    NodeEvalState nes;

    EXPECT_EQ(IB_PHASE_NONE, nes.phase());
    nes.set_phase(IB_PHASE_REQUEST_HEADER);
    EXPECT_EQ(IB_PHASE_REQUEST_HEADER, nes.phase());
}

TEST_F(TestEval, NodeEvalState_State)
{
    NodeEvalState nes;
    int i = 5;

    EXPECT_TRUE(nes.state().empty());
    nes.state() = i;
    EXPECT_FALSE(nes.state().empty());
    EXPECT_EQ(i, boost::any_cast<int>(nes.state()));
}

TEST_F(TestEval, GraphEvalState)
{
    node_p n0(new Literal);
    node_p n1(new Literal);
    node_p n2(new Literal);
    node_p n3(new Literal);
    node_p n4(new Literal("Hello World"));

    GraphEvalState ges(5);
    NodeEvalState& local = ges.node_eval_state(0);
    NodeEvalState& alias = ges.node_eval_state(1);
    NodeEvalState& forwarded = ges.node_eval_state(2);
    NodeEvalState& forwarded2 = ges.node_eval_state(3);

    n0->set_index(0);
    n1->set_index(1);
    n2->set_index(2);
    n3->set_index(3);
    n4->set_index(4);

    forwarded2.forward(n2.get());
    forwarded.forward(n4.get());

    IronBee::ScopedMemoryPoolLite mpl;
    IronBee::Field f = IronBee::Field::create_number(mpl, "", 0, 5);
    Value v(f);

    alias.alias(v);
    alias.finish();

    local.setup_local_list(m_transaction.memory_manager());

    EXPECT_EQ(&ges.node_eval_state(0), &ges.index_final(n0->index()));
    EXPECT_EQ(&ges.node_eval_state(1), &ges.index_final(n1->index()));
    EXPECT_EQ(&ges.node_eval_state(4), &ges.index_final(n2->index()));
    EXPECT_EQ(&ges.node_eval_state(4), &ges.index_final(n3->index()));
    EXPECT_EQ(&ges.node_eval_state(4), &ges.index_final(n4->index()));

    ges.initialize(n4.get(), m_transaction);
    ges.eval(n3.get(), m_transaction);
    Value result = ges.value(n3.get(), m_transaction);

    EXPECT_TRUE(result);
    EXPECT_EQ("'Hello World'", result.to_s());

    EXPECT_FALSE(ges.index_final(n0->index()).value());
    EXPECT_TRUE(ges.index_final(n1->index()).value());
    EXPECT_TRUE(ges.index_final(n2->index()).value());
    EXPECT_TRUE(ges.index_final(n3->index()).value());
    EXPECT_TRUE(ges.index_final(n4->index()).value());

    EXPECT_FALSE(ges.index_final(n0->index()).is_finished());
    EXPECT_TRUE(ges.index_final(n1->index()).is_finished());
    EXPECT_TRUE(ges.index_final(n2->index()).is_finished());
    EXPECT_TRUE(ges.index_final(n3->index()).is_finished());
    EXPECT_TRUE(ges.index_final(n4->index()).is_finished());
}
