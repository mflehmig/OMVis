/*
 * Copyright (C) 2016, Volker Waurich
 *
 * This file is part of OMVis.
 *
 * OMVis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OMVis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OMVis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEST_INCLUDE_TESTOMVISUALIZERFMU_HPP_
#define TEST_INCLUDE_TESTOMVISUALIZERFMU_HPP_

#include "Model/OMVisualizerFMU.hpp"
#include "Control/OMVisManager.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <iostream>

class TestOMVisualizerFMU : public TestCommon
{
 public:

    OMVIS::Model::OMVisualizerFMU* _omVisualizerFMU;
    TestOMVisualizerFMU()
            : TestCommon("BouncingBall", "./data/BouncingBall/", true),
              _omVisualizerFMU(nullptr)
    {
    }

    void SetUp()
    {
        _omVisualizerFMU = new OMVIS::Model::OMVisualizerFMU(_modelName, _path);
    }

    void TearDown()
    {
    }

    ~TestOMVisualizerFMU()
    {
    }
};

TEST_F (TestOMVisualizerFMU, TestLoad)
{
    // Not nullptr.
    ASSERT_TRUE(_omVisualizerFMU);
    ASSERT_TRUE(_omVisualizerFMU->_baseData);
    ASSERT_TRUE(_omVisualizerFMU->_viewerStuff);
    ASSERT_TRUE(_omVisualizerFMU->_nodeUpdater);
    ASSERT_TRUE(_omVisualizerFMU->_omvManager);

    // Knows its type.
    ASSERT_EQ("fmu", _omVisualizerFMU->getType());

    // FMU is not yet unzipped.
    const OMVIS::Model::FMU* fmu = _omVisualizerFMU->getFMU();
    ASSERT_FALSE(fmu->isUnzipped());
}

TEST_F (TestOMVisualizerFMU, TestInitialization)
{    // Initialize.
    int isOK = _omVisualizerFMU->initialize();
    ASSERT_EQ(0, isOK);

    // Check FMU.
    const OMVIS::Model::FMU* fmu = _omVisualizerFMU->getFMU();
    ASSERT_TRUE(fmu->_fmu);
    ASSERT_TRUE(fmu->_context);
    ASSERT_TRUE(fmu->_callbacks);
    ASSERT_TRUE(fmu->isUnzipped());

    // Check FMUData.
    const OMVIS::Model::FMUData* fmuData = fmu->getFMUData();
    ASSERT_EQ(2, fmuData->_nStates);
    ASSERT_EQ(2, fmuData->_nEventIndicators);

    // Check OMVisManager.
    OMVIS::Control::OMVisManager* omvManager = _omVisualizerFMU->_omvManager;
    ASSERT_EQ(0.0, omvManager->getStartTime());
    ASSERT_EQ(0.0, omvManager->getVisTime());
    ASSERT_EQ(0.0, omvManager->getSimTime());
    ASSERT_EQ(0.0, omvManager->getRealTime());
    ASSERT_EQ(100.0, omvManager->getEndTime());
    ASSERT_EQ(0.1, omvManager->getHVisual());
    ASSERT_TRUE(omvManager->isPaused());
}

#endif /* TEST_INCLUDE_TESTOMVISUALIZERFMU_HPP_ */
