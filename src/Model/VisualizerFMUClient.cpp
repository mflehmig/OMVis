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

#include "Model/VisualizerFMUClient.hpp"
#include "Util/Logger.hpp"

namespace OMVIS
{
    namespace Model
    {

        /*-----------------------------------------
         * CONSTRUCTORS
         *---------------------------------------*/

        VisualizerFMUClient::VisualizerFMUClient(const Initialization::RemoteVisualizationConstructionPlan* cP)
                : VisualizerAbstract(cP->modelFile, cP->wDir, VisType::FMU_REMOTE),
                  _noFC(cP->hostAddress, cP->port),
                  _simID(-1),
                  _simSettings(std::make_shared<SimSettingsFMU>()),
                  _outputVars(),
                  _inputData(std::make_shared<InputData>()),
                  _joysticks(),
                  _remotePathToModelFile(cP->path)
        {
            LOGGER_WRITE("Initialize joysticks", Util::LC_LOADER, Util::LL_INFO);
            initJoySticks();
        }

        VisualizerFMUClient::~VisualizerFMUClient()
        {
            LOGGER_WRITE("Calling NetOff::SimulationClient::deinitialize() ... ", Util::LC_LOADER, Util::LL_DEBUG);
            _noFC.deinitialize();
            LOGGER_WRITE("SimulationClient successfully deinitialized. ", Util::LC_LOADER, Util::LL_DEBUG);
        }

        /*-----------------------------------------
         * INITIALIZATION METHODS
         *---------------------------------------*/

        void VisualizerFMUClient::initialize()
        {
            // If visual XML file is not present, we need to copy it from server to localhost
            // (before calling VisualizerAbstract::initialize().

            try
            {
                initializeConnectionToServer();
            }
            catch (std::exception& ex)
            {
                auto msg = "VisualizerFMUClient exception: " + std::string(ex.what());
                LOGGER_WRITE(msg, Util::LC_LOADER, Util::LL_ERROR);
                throw std::runtime_error(msg);
            }

            VisualizerAbstract::initialize();
        }

        void VisualizerFMUClient::initializeConnectionToServer()
        {
            // Test if server can be reached
            if (!_noFC.initializeConnection())
            {
                throw std::runtime_error("Timeout: Cannot connect to server.");
            }
        }

        void VisualizerFMUClient::loadFMU()
        {
            auto a = _baseData->getModelFile();
            // Send server data about the simulation you want to calculate and get the ID of the simulation.
            //_simID = _noFC.addSimulation(_baseData->getModelName());
            _simID = _noFC.addSimulation(a);

            // Set visualization relevant output variables.
            NetOff::VariableList outputVars = getOutputVariables();

            // Set input variables. Todo: Open GUI window.
            NetOff::VariableList inputVars = getInputVariables();
            _inputData->initializeInputs(inputVars);
            _inputData->printValues();

            _outputVars = getOutputVariables();

            // Communicate input and output variables with server.
            _noFC.initializeSimulation(_simID, inputVars, outputVars, nullptr, nullptr, nullptr);
        }

        void VisualizerFMUClient::initData()
        {
            VisualizerAbstract::initData();
            loadFMU();
            _simSettings->setTend(_timeManager->getEndTime());
            _simSettings->setHdef(0.001);
            setVarReferencesInVisAttributes();

            //OMVisualizerFMU::initializeVisAttributes(_omvManager->getStartTime());
        }

        void VisualizerFMUClient::resetInputs()
        {
            _inputData->resetInputValues();
        }

        void VisualizerFMUClient::initJoySticks()
        {
            //Initialize SDL
            if (0 > SDL_Init(SDL_INIT_JOYSTICK))
            {
                LOGGER_WRITE("SDL could not be initialized.", Util::LC_LOADER, Util::LL_ERROR);
            }

            //Check for joysticks
            if (1 > SDL_NumJoysticks())
            {
                LOGGER_WRITE("No joysticks connected!", Util::LC_LOADER, Util::LL_WARNING);
            }
            else
            {
                LOGGER_WRITE("Found " + std::to_string(SDL_NumJoysticks()) + " joystick(s)", Util::LC_LOADER,
                             Util::LL_INFO);

                //Load joystick
                LOGGER_WRITE("START LOADING JOYSTICKS!!!!!!!!!", Util::LC_LOADER, Util::LL_INFO);
                Control::JoystickDevice* newJoyStick;
                for (int i = 0; i < SDL_NumJoysticks(); ++i)
                {
                    LOGGER_WRITE("LOAD JOYSTICK # " + std::to_string(i), Util::LC_LOADER, Util::LL_INFO);
                    newJoyStick = new Control::JoystickDevice(i);
                    _joysticks.push_back(newJoyStick);

                    if (nullptr == newJoyStick)
                    {
                        LOGGER_WRITE("Unable to open joystick! SDL Error: " + std::string(SDL_GetError()),
                                     Util::LC_LOADER, Util::LL_INFO);
                    }
                }
            }
        }

        /*-----------------------------------------
         * GETTERS and SETTERS
         *---------------------------------------*/

        NetOff::VariableList VisualizerFMUClient::getOutputVariables()
        {
            NetOff::VariableList varList;
            varList.addReals(_baseData->getVisualizationVariables());
            return varList;
        }

        // Todo! Implement me. Use a GUI to set input values.
        NetOff::VariableList VisualizerFMUClient::getInputVariables()
        {
            NetOff::VariableList varList;
            varList.addReals(_noFC.getPossibleInputVariableNames(_simID).getReals());
            return varList;
        }

        std::shared_ptr<InputData> VisualizerFMUClient::getInputData()
        {
            return _inputData;
        }

        void VisualizerFMUClient::setSimulationSettings(const UserSimSettingsFMU& simSetFMU)
        {
            if (Solver::NONE == simSetFMU.solver)
            {
                throw std::runtime_error("Solver: NONE is not a valid solver.");
            }
            else
            {
                _simSettings->setSolver(simSetFMU.solver);
            }

            if (0.0 >= simSetFMU.simStepSize)
            {
                throw std::runtime_error(
                        "Simulation step size of " + std::to_string(simSetFMU.simStepSize) + " is not valid.");
            }
            else
            {
                _simSettings->setHdef(simSetFMU.simStepSize);
            }

            if (0.0 >= simSetFMU.simEndTime)
            {
                throw std::runtime_error("Solver: Simulation end time <= 0.0 is not valid.");
            }
            else
            {
                _timeManager->setEndTime(simSetFMU.simEndTime);
            }
        }

        UserSimSettingsFMU VisualizerFMUClient::getCurrentSimSettings() const
        {
            return
            {   _simSettings->getSolver(), _simSettings->getHdef(), 99, _timeManager->getEndTime()};
        }

        /*-----------------------------------------
         * SIMULATION METHODS
         *---------------------------------------*/

        void VisualizerFMUClient::simulate(Control::TimeManager& omvm)
        {
            double newSimTime;
            while (omvm.getSimTime() < omvm.getRealTime() + omvm.getHVisual() && omvm.getSimTime() < omvm.getEndTime())
            {
                newSimTime = simulateStep(omvm.getSimTime());
                omvm.setSimTime(newSimTime);
            }
        }

        double VisualizerFMUClient::simulateStep(const double time)
        {
            double newTime = time + _simSettings->getHdef();

            NetOff::ValueContainer& inputCont = _noFC.getInputValueContainer(_simID);
            // Set inputs in inputData
            for (auto& joystick : _joysticks)
            {
                joystick->detectContinuousInputEvents(_inputData);

                //_inputData->setInputsInFMU(_fmu->getFMU()); //todo aus der schleife raus???
                //std::cout << "JOY" << i << " XDir " <<_joysticks[i]->getXDir() <<" YDir "<< _joysticks[i]->getYDir() << std::endl;
            }

            // Set inputs for network communication.
            inputCont.setRealValues(_inputData->getRealValues());
            inputCont.setIntValues(_inputData->getIntValues());
            inputCont.setBoolValues(_inputData->getBoolValues());

            // Send input values to server
            _noFC.sendInputValues(_simID, newTime, inputCont);
            // Receive output values from server for visualisation
            _noFC.recvOutputValues(_simID, newTime);  // implicit check if its really [time]
            return newTime;
        }

        void VisualizerFMUClient::initializeVisAttributes(const double /*time*/)
        {
            _timeManager->setVisTime(_timeManager->getStartTime());
            _timeManager->setSimTime(_timeManager->getStartTime());
            setVarReferencesInVisAttributes();
            updateVisAttributes(_timeManager->getVisTime());
        }

        fmi1_value_reference_t VisualizerFMUClient::getVarReferencesForObjectAttribute(ShapeObjectAttribute* attr)
        {
            fmi1_value_reference_t vr = 0;
            if (!attr->isConst)
            {
                vr = _outputVars.findRealVariableNameIndex(attr->cref);
            }
            return vr;
        }

        int VisualizerFMUClient::setVarReferencesInVisAttributes()
        {
            int isOk(0);

            try
            {
                for (auto& shape : _baseData->_shapes)
                {
                    shape._length.fmuValueRef = getVarReferencesForObjectAttribute(&shape._length);
                    shape._width.fmuValueRef = getVarReferencesForObjectAttribute(&shape._width);
                    shape._height.fmuValueRef = getVarReferencesForObjectAttribute(&shape._height);

                    shape._lDir[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._lDir[0]);
                    shape._lDir[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._lDir[1]);
                    shape._lDir[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._lDir[2]);

                    shape._wDir[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._wDir[0]);
                    shape._wDir[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._wDir[1]);
                    shape._wDir[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._wDir[2]);

                    shape._r[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._r[0]);
                    shape._r[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._r[1]);
                    shape._r[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._r[2]);

                    shape._rShape[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._rShape[0]);
                    shape._rShape[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._rShape[1]);
                    shape._rShape[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._rShape[2]);

                    shape._T[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[0]);
                    shape._T[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[1]);
                    shape._T[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[2]);
                    shape._T[3].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[3]);
                    shape._T[4].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[4]);
                    shape._T[5].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[5]);
                    shape._T[6].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[6]);
                    shape._T[7].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[7]);
                    shape._T[8].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[8]);
                }  //end for
            }  // end try

            catch (std::exception& e)
            {
                LOGGER_WRITE("Something went wrong in Visualizer::setVarReferencesInVisAttributes", Util::LC_SOLVER,
                             Util::LL_WARNING);
                isOk = 1;
            }
            return isOk;
        }

        void VisualizerFMUClient::updateVisAttributes(const double time)
        {
            // Update all shapes.
            OMVIS::Util::rAndT rT;
            NetOff::ValueContainer & outputCont = _noFC.getOutputValueContainer(_simID);
            osg::ref_ptr<osg::Node> child = nullptr;
            try
            {
                size_t i = 0;
                for (auto& shape : _baseData->_shapes)
                {
                    // get the values for the scene graph objects
                    Util::updateObjectAttributeFMUClient(shape._length, outputCont);
                    Util::updateObjectAttributeFMUClient(shape._width, outputCont);
                    Util::updateObjectAttributeFMUClient(shape._height, outputCont);

                    Util::updateObjectAttributeFMUClient(shape._lDir[0], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._lDir[1], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._lDir[2], outputCont);

                    Util::updateObjectAttributeFMUClient(shape._wDir[0], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._wDir[1], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._wDir[2], outputCont);

                    Util::updateObjectAttributeFMUClient(shape._r[0], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._r[1], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._r[2], outputCont);

                    Util::updateObjectAttributeFMUClient(shape._rShape[0], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._rShape[1], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._rShape[2], outputCont);

                    Util::updateObjectAttributeFMUClient(shape._T[0], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[1], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[2], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[3], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[4], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[5], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[6], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[7], outputCont);
                    Util::updateObjectAttributeFMUClient(shape._T[8], outputCont);
                    rT = Util::rotation(
                            osg::Vec3f(shape._r[0].exp, shape._r[1].exp, shape._r[2].exp),
                            osg::Vec3f(shape._rShape[0].exp, shape._rShape[1].exp, shape._rShape[2].exp),
                            osg::Matrix3(shape._T[0].exp, shape._T[1].exp, shape._T[2].exp, shape._T[3].exp,
                                         shape._T[4].exp, shape._T[5].exp, shape._T[6].exp, shape._T[7].exp,
                                         shape._T[8].exp),
                            osg::Vec3f(shape._lDir[0].exp, shape._lDir[1].exp, shape._lDir[2].exp),
                            osg::Vec3f(shape._wDir[0].exp, shape._wDir[1].exp, shape._wDir[2].exp), shape._length.exp,
                            shape._type);

                    Util::assemblePokeMatrix(shape._mat, rT._T, rT._r);

                    // Update the shapes.
                    _nodeUpdater->_shape = shape;

                    // Get the scene graph nodes and stuff.
                    //_viewerStuff->dumpOSGTreeDebug();
                    child = _viewerStuff->getScene()->getRootNode()->getChild(i);  // the transformation
                    child->accept(*_nodeUpdater);
                    ++i;
                }  //end for
            }  // end try

            catch (std::exception& ex)
            {
                auto msg = "Error in VisualizerFMUClient::updateVisAttributes at time point " + std::to_string(time)
                        + "\n" + std::string(ex.what());
                LOGGER_WRITE(msg, Util::LC_SOLVER, Util::LL_WARNING);
            }
        }

        void VisualizerFMUClient::startVisualization()
        {
            if (!_noFC.isStarted())
            {
                _noFC.start();
            }
            else
            {
                _noFC.unpause();
            }
            VisualizerAbstract::startVisualization();
        }

        void VisualizerFMUClient::pauseVisualization()
        {
            _noFC.pause();
            VisualizerAbstract::pauseVisualization();
        }

        void VisualizerFMUClient::updateScene(const double /*time*/)
        {
            _timeManager->updateTick();            //for real-time measurement

            _timeManager->setSimTime(_timeManager->getVisTime());
            double nextStep = _timeManager->getVisTime() + _timeManager->getHVisual();

            double vis1 = _timeManager->getRealTime();
            while (_timeManager->getSimTime() < nextStep)
            {
                //std::cout<<"simulate "<<omvManager->_simTime<<" to "<<nextStep<<std::endl;
                //_inputData.printValues();
                _timeManager->setSimTime(simulateStep(_timeManager->getSimTime()));
            }
            _timeManager->updateTick();                     //for real-time measurement
            _timeManager->setRealTimeFactor(_timeManager->getHVisual() / (_timeManager->getRealTime() - vis1));
            updateVisAttributes(_timeManager->getVisTime());
        }

    }  // namespace Model
}  // namespace OMVIS
