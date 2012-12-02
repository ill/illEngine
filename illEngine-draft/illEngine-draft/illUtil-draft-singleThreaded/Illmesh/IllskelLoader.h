#ifndef __ILLSKELLOADER_H__
#define __ILLSKELLOADER_H__

#include <string>
#include "../Logging/logging.h"
#include "../physFsUtil.h"     //TODO: temporary

/**
Loads the amazing illskel ASCII format.
*/
template <typename T = glm::mediump_float>
struct IllmeshLoader {
    IllmeshLoader(const std::string& fileName) {
        //TODO: LOL this is horrible, inefficient, and temporary, and works just fine in most situations so whatever...
        //I need to write a nice file IO interface around physfs for PC and whatever we need on Android and Ios when the time comes, for now this will work
        char * objText;

        readFile(fileName, &objText);
        m_openFile = new std::stringstream(std::string(objText), std::ios_base::in);        
        delete[] objText;

        //read header
        {
            std::string magicStr;

            (*m_openFile) >> magicStr;

            if(magicStr.compare("ILLSKEL0") != 0) {
                LOG_FATAL_ERROR("Not a valid ILLSKEL0 file.");      //TODO: make this not fatal somehow, I guess all meshes would be in their rest pose if they're supposed to have an associated skeleton
            }
        }

        (*m_openFile) >> m_numBones;
    }

    unsigned int m_numBones;
    std::stringstream * m_openFile;
};

#endif