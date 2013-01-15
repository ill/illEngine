#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/serial/RendererFrontend.h"
#include "Graphics/RendererBackend.h"
#include "Graphics/serial/BitmapFont.h"

#include "Util/Geometry/geomUtil.h"

namespace illGraphics {

void RendererFrontend::renderText(const char * text, const glm::mat4& transform, const BitmapFont& font, const Camera& camera, FontHorzAlign horzAlign, FontVertAlign vertAlign) {
    glm::vec4 currColor = glm::vec4(1.0f);  //set the color to white initially

    glm::mat4 currentTransform;

    if(horzAlign != FN_H_LEFT || vertAlign != FN_V_TOP)  {
        glm::vec2 printDimensions = font.getPrintDimensions(text);
        glm::vec2 translate;

        switch (vertAlign) {
        case FN_V_CENTER:
            translate.y = printDimensions.y * 0.5f;            
            break;

        case FN_V_BOTTOM:
            translate.y = printDimensions.y;
            break;

        default:
            translate.y = 0;
            break;
        }

        switch (horzAlign) {
        case FN_H_CENTER: {
                const char ** textPtr = &text;
                translate.x = font.getLineWidth(textPtr) * 0.5f;
            }
            break;

        case FN_H_RIGHT: {
                const char ** textPtr = &text;
                translate.x = font.getLineWidth(textPtr);
            }
            break;

        default:
            translate.x = 0;
        }

        currentTransform = glm::translate(transform, glm::vec3(translate, 0.0f));
    }

    while(*text) {
        //check if color code
        if(font.getColorCode(&text, currColor)) {
            continue;
        }

        //parse special characters
        switch (*text) {
        case '\n': {    //newline
            glm::vec2 translate;
            
            text++;

            glm::vec3 currentPosition = getTransformPosition(currentTransform);

            switch (horzAlign) {
            case FN_H_CENTER: {
                    const char ** textPtr = &text;
                    translate.x = font.getLineWidth(textPtr) * 0.5f;
                }
                break;

            case FN_H_RIGHT: {
                    const char ** textPtr = &text;
                    translate.x = font.getLineWidth(textPtr);
                }
                break;

            default:
                translate.x = 0;
            }

            translate.y = currentPosition.y + font.getLineHeight();

            currentTransform = glm::translate(transform, glm::vec3(translate, 0.0f));

            continue;
        }

        case ' ': //space
            currentTransform = glm::translate(transform, glm::vec3(font.getSpacingHorz(), 0.0f, 0.0f));
            text++;
            continue;

        case '\t': //tab
            currentTransform = glm::translate(transform, glm::vec3(font.getSpacingHorz() * 4.0f, 0.0f, 0.0f));
            text++;
            continue;
        }

        //m_backend->renderCharacter(camera, transform, font, currColor, *text);
        currentTransform = glm::translate(transform, glm::vec3(font.getCharData(*text).m_advance, 0.0f, 0.0f));
        text++;
    }
}

}