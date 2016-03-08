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

/*
 * GUIController.hpp
 *
 *  Created on: 01.03.2016
 *      Author: mf
 */

#ifndef INCLUDE_CONTROLLER_GUICONTROLLER_HPP_
#define INCLUDE_CONTROLLER_GUICONTROLLER_HPP_

// Forward declaration
class OMVisualizerAbstract;


namespace Controller
{

    /*! \brief This class is a controller class which handles user interaction with OMVis via GUI.
     *
     * The GUIcontroller receives the user input (,e.g., clicking a menu point), process it and induces
     * the appropriate action.
     */
    class GUIController
    {
     public:
        GUIController() = default;
        ~GUIController() = default;

        GUIController(const GUIController& gc) = delete;
        GUIController& operator=(const GUIController& gc) = delete;

        /*! \brief This method gets the name and path of a model which should be loaded into OMVIS.
         *
         * This method parses the given string for model name, path to the model file and whether it is a FMU or a
         * MAT file and asks the factory for creation of an appropriate OMVisualizer object. Furthermore, the
         * OMVisualizer object is initialized.
         * The method checks, if the corresponding XML file is accessible.
         * \return Pointer to OMVisualizer object which is created by the factory.
         * \remark We assume that the XML file is located in the very same directory as the FMU or MAT file.
         */
        Model::OMVisualizerAbstract* loadModel(const std::string& modelName);

        /*! \brief Check if the XML file for a given path and model name can be accessed.
         *
         * The XML file name is created by "path + modelName + "_visual.xml"". Thus, the XML file has to be renamed
         * if it does not fit this convention.
         *
         * @param path Path to the XML file.
         * @param modelName Name of the model.
         * @return True, if the XML file is accessible. False, otherwise.
         */
        bool checkForXMLFile(const std::string& path, const std::string& modelName);

     private:
        // No members
    };

}  // End namespace Controller

#endif /* INCLUDE_CONTROLLER_GUICONTROLLER_HPP_ */
