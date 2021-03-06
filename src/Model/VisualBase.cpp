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

#include "Model/VisualBase.hpp"
#include "Util/Logger.hpp"
#include "Util/Util.hpp"

#include <osgDB/ReadFile>

#include <exception>

namespace OMVIS
{
    namespace Model
    {

        /*-----------------------------------------
         * CONSTRUCTORS
         *---------------------------------------*/

        VisualBase::VisualBase(const std::string& modelFile, const std::string& path)
                : _modelFile(modelFile),
                  _path(path),
                  _xmlDoc(),
                  _shapes(),
                  _xmlFileName(Util::getXMLFileName(modelFile, path))
        {
        }

        /*-----------------------------------------
         * INITIALIZATION METHODS
         *---------------------------------------*/

        void VisualBase::initXMLDoc()
        {
            // Check if the XML file is available.
            if (!Util::fileExists(_xmlFileName))
            {
                auto msg = "VisualBase: Could not find the visxml file" + _xmlFileName + ".";
                LOGGER_WRITE(msg, Util::LC_LOADER, Util::LL_ERROR);
                throw std::runtime_error(msg);
            }
            // read xml
            osgDB::ifstream t;
            // unused const char * titel = _xmlFileName.c_str();
            t.open(_xmlFileName.c_str(), std::ios::binary);      // open input file
            t.seekg(0, std::ios::end);    // go to the end
            int length = t.tellg();       // report location (this is the length)
            t.seekg(0, std::ios::beg);    // go back to the beginning
            auto buffer = new char[length];    // allocate memory for a buffer of appropriate dimension
            t.read(buffer, length);       // read the whole file into the buffer
            t.close();
            std::string buff = std::string(buffer);  // strings are good
            std::string buff2 = buff.substr(0, buff.find("</visualization>"));  // remove the crappy ending
            buff2.append("</visualization>");
            char* buff3 = strdup(buff2.c_str());  // cast to char*
            _xmlDoc.parse<0>(buff3);
            LOGGER_WRITE("Reading the visxml file " + _xmlFileName + " was successful.", Util::LC_LOADER,
                         Util::LL_DEBUG);
        }

        /// \todo Can we call std::vector<T>::reserve before pushing back all shapes?
        void VisualBase::initVisObjects()
        {
            auto rootNode = _xmlDoc.first_node();
            rapidxml::xml_node<>* expNode;
            Model::ShapeObject shape;

            //Begin std::vector<T>::reserve()
            //int i = 0;
            //for (rapidxml::xml_node<>* shapeNode = rootNode->first_node("shape"); shapeNode; shapeNode = shapeNode->next_sibling())
            //    ++i;
            //LOGGER_WRITE(std::string("============Number of iterations1 ") + std::to_string(i), Util::LC_LOADER, Util::LL_DEBUG);
            //_shapes.reserve(i);
            // End std::vector<T>::reserve()

            for (auto shapeNode = rootNode->first_node("shape"); nullptr != shapeNode;
                    shapeNode = shapeNode->next_sibling())
            {
                expNode = shapeNode->first_node("ident")->first_node();
                shape._id = std::string(expNode->value());

//                expNode = shapeNode->first_node((const char*) "type")->first_node();
                expNode = shapeNode->first_node("type")->first_node();

                if (nullptr == expNode)
                {
                    LOGGER_WRITE("The type of  " + shape._id + " is not supported right in the visxml file.",
                                 Util::LC_LOADER, Util::LL_DEBUG);
                }
                else
                {
                    shape._type = std::string(expNode->value());
                    if (Util::isCADType(shape._type))
                    {
                        shape._fileName = Util::extractCADFilename(shape._type);
                        if (Util::dxfFileType(shape._fileName))
                        {
                            shape._type = "dxf";
                        }
                        else if (Util::stlFileType(shape._fileName))
                        {
                            shape._type = "stl";
                        }
                        if (!Util::fileExists(shape._fileName))
                        {
                            auto msg = "VisualBase: Could not find the file " + shape._fileName + ".";
                            LOGGER_WRITE(msg, Util::LC_LOADER, Util::LL_DEBUG);
                            throw std::runtime_error(msg);
                        }
                    }
                    //std::cout<<"type "<<shape._id<<std::endl;
                    //std::cout<<"type "<<shape._type<<std::endl;

                    expNode = shapeNode->first_node("length")->first_node();
                    shape._length = Util::getObjectAttributeForNode(expNode);
                    expNode = shapeNode->first_node("width")->first_node();
                    shape._width = Util::getObjectAttributeForNode(expNode);
                    expNode = shapeNode->first_node("height")->first_node();
                    shape._height = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("lengthDir")->first_node();
                    shape._lDir[0] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._lDir[1] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._lDir[2] = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("widthDir")->first_node();
                    shape._wDir[0] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._wDir[1] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._wDir[2] = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("r")->first_node();
                    shape._r[0] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._r[1] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._r[2] = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("r_shape")->first_node();
                    shape._rShape[0] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._rShape[1] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._rShape[2] = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("color")->first_node();
                    shape._color[0] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._color[1] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._color[2] = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("T")->first_node();
                    shape._T[0] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[1] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[2] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[3] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[4] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[5] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[6] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[7] = Util::getObjectAttributeForNode(expNode);
                    expNode = expNode->next_sibling();
                    shape._T[8] = Util::getObjectAttributeForNode(expNode);

                    expNode = shapeNode->first_node("specCoeff")->first_node();
                    shape._specCoeff = Util::getObjectAttributeForNode(expNode);
                    expNode = shapeNode->first_node("extra")->first_node();
                    shape._extra = Util::getObjectAttributeForNode(expNode);

                    _shapes.push_back(shape);
                }
            }  // end for-loop

            //std::vector<std::string> vs = getVisualizationVariables();
        }

        void VisualBase::clearXMLDoc()
        {
            _xmlDoc.clear();
        }

        /*-----------------------------------------
         * GETTERS and SETTERS
         *---------------------------------------*/

        rapidxml::xml_node<>* VisualBase::getFirstXMLNode() const
        {
            return _xmlDoc.first_node();
        }

        const std::string VisualBase::getModelFile() const
        {
            return _modelFile;
        }

        const std::string VisualBase::getPath() const
        {
            return _path;
        }

        const std::string VisualBase::getXMLFileName() const
        {
            return _xmlFileName;
        }

        void VisualBase::appendVisVariable(const rapidxml::xml_node<>* node,
                                             std::vector<std::string>& visVariables) const
        {
            if (0 == strcmp("cref", node->name()))
            {
                char* cref = node->value();
                visVariables.push_back(std::string(cref));
            }
        }

        std::vector<std::string> VisualBase::getVisualizationVariables() const
        {
            auto rootNode = _xmlDoc.first_node();
            rapidxml::xml_node<>* expNode;
            Model::ShapeObject shape;

            // Oh yes, I know how large this vector will be.
            std::vector<std::string> visVariables;
            visVariables.reserve(_shapes.size());

            for (auto shapeNode = rootNode->first_node("shape"); shapeNode != nullptr;
                    shapeNode = shapeNode->next_sibling())
            {
                expNode = shapeNode->first_node("ident")->first_node();
                shape._id = std::string(expNode->value());

                expNode = shapeNode->first_node("type")->first_node();
                if (expNode == nullptr)
                {
                    LOGGER_WRITE("The type of  " + shape._id + " is not supported right in the visxml file.",
                                 Util::LC_LOADER, Util::LL_DEBUG);
                    break;
                }

                shape._type = std::string(expNode->value());

                expNode = shapeNode->first_node("length")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = shapeNode->first_node("width")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = shapeNode->first_node("height")->first_node();
                appendVisVariable(expNode, visVariables);

                expNode = shapeNode->first_node("lengthDir")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);

                expNode = shapeNode->first_node("widthDir")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);

                expNode = shapeNode->first_node("r")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);

                expNode = shapeNode->first_node("r_shape")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);

                expNode = shapeNode->first_node("color")->first_node();
                shape._color[0] = Util::getObjectAttributeForNode(expNode);
                expNode = expNode->next_sibling();
                shape._color[1] = Util::getObjectAttributeForNode(expNode);
                expNode = expNode->next_sibling();
                shape._color[2] = Util::getObjectAttributeForNode(expNode);

                expNode = shapeNode->first_node("T")->first_node();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);
                expNode = expNode->next_sibling();
                appendVisVariable(expNode, visVariables);

                expNode = shapeNode->first_node("specCoeff")->first_node();
                appendVisVariable(expNode, visVariables);
            }
            LOGGER_WRITE("There are " + std::to_string(visVariables.size()) + " visualization variables.",
                         Util::LC_LOADER, Util::LL_INFO);
            return visVariables;
        }

    }  // namespace Model
}  // namespace OMVIS
