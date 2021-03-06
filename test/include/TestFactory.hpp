/*
 * Copyright (C) 2016, Volker Waurich
 *
 * This file is part of OMVIS.
 *
 * OMVIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OMVIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OMVIS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEST_INCLUDE_TESTFACTORY_HPP_
#define TEST_INCLUDE_TESTFACTORY_HPP_

#include "TestCommon.hpp"

#include "Initialization/Factory.hpp"
#include "Model/VisualizerAbstract.hpp"
#include "Model/VisualizerFMU.hpp"

#include <gtest/gtest.h>


/*! \brief Class to test the factory of OMVIS \ref OMVIS::Initialization::Factory.
 *
 * The test fixtures test that the factory creates the right "product", i.e., if we visualize a
 * FMU we want a VisualizerFMU object to be created and if we visualize a MAT file we want a
 * VisualizerMAT object to be created. The default case is to create VisualizerAbstract object.
 */
class TestOMVisFactory : public TestCommon
{
 public:
    OMVIS::Initialization::Factory* _factory;
    std::shared_ptr<OMVIS::Model::VisualizerAbstract> _omvisualizerAbstract;

    TestOMVisFactory()
            : TestCommon("BouncingBall.fmu", "./examples/"),
              _factory(new OMVIS::Initialization::Factory()),
              _omvisualizerAbstract(nullptr)
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    ~TestOMVisFactory()
    {
    }
};

/*! Test the creation of an VisualizerFMU object by the factory. */
TEST_F (TestOMVisFactory, CreateOMVisualizerFMU)
{
    // FMU.
    _omvisualizerAbstract = _factory->createVisualizerObject(constructionPlan.get());
    EXPECT_EQ(OMVIS::Model::VisType::FMU, _omvisualizerAbstract->getVisType());
}

/*! Test the creation of an VisualizerMAT object by the factory. */
TEST_F (TestOMVisFactory, CreateOMVisualizerMAT)
{
    // MAT.
    reset("pendulum_res.mat", "./examples/");
    _omvisualizerAbstract = _factory->createVisualizerObject(constructionPlan.get());
    EXPECT_EQ(OMVIS::Model::VisType::MAT, _omvisualizerAbstract->getVisType());
}

#endif /* TEST_INCLUDE_TESTFACTORY_HPP_ */

