#include "terrain.h"

#include <math.h>
#include "gl/shaders/ShaderAttribLocations.h"

Terrain::Terrain() : m_numRows(100), m_numCols(m_numRows)
{
}


/**
 * Returns a pseudo-random value between -1.0 and 1.0 for the given row and column.
 */
float Terrain::randValue(int row, int col) {
    return -1.0 + 2.0 * glm::fract(sin(row * 127.1f + col * 311.7f) * 43758.5453123f);
}


/**
 * Returns the object-space position for the terrain vertex at the given row and column.
 */
glm::vec3 Terrain::getPosition(int row, int col) {
    glm::vec3 position;
    position.x = 10 * row/m_numRows - 5;

    //new row on the 5x5 grid
    int newRow = floor(row/20.f);
    int newCol = floor(col/20.f);

    int newRow2 = floor(row/10.f);
    int newCol2 = floor(col/10.f);

    position.y = randValue(newRow, newCol);
    position.z = 10 * col/m_numCols - 5;

    // TODO: Adjust position.y using value noise.




    float tl = randValue(newRow, newCol);
    float tr = randValue(newRow, newCol+1);
    float bl = randValue(newRow+1, newCol);
    float br = randValue(newRow+1, newCol+1);


    float tl2 = randValue(newRow2, newCol2);
    float tr2 = randValue(newRow2, newCol2+1);
    float bl2 = randValue(newRow2+1, newCol2);
    float br2 = randValue(newRow2+1, newCol2+1);

//    (1.f - glm::fract(row/20.f));
//    (1.f - glm::fract(col/20.f));

    float x1 = (glm::fract(row/20.f));
    float x2 = (glm::fract(col/20.f));


    float t1 = glm::mix(tl, tr, x2*x2*(3-2*x2));
    float t3 = glm::mix(bl, br, x2*x2*(3-2*x2));



    float x12 = (glm::fract(row/10.f));
    float x22 = (glm::fract(col/10.f));


    float t12 = glm::mix(tl2, tr2, x22*x22*(3-2*x22));
    float t32 = glm::mix(bl2, br2, x22*x22*(3-2*x22));



//    float centerDist = fabs(glm::fract(row/20.f)-0.5f) + fabs(glm::fract(col/20.f)-0.5f);



    //position.y = glm::mix(glm::mix(one, two, (glm::fract(col/20.f))), position.y, centerDist);
    position.y = glm::mix(t1, t3, x1*x1*(3-2*x1)) + glm::mix(t12, t32, x12*x12*(3-2*x12));


    return position;
}



//function to add three vectors together
glm::vec3 average8Vectors(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, glm::vec3 v5, glm::vec3 v6, glm::vec3 v7, glm::vec3 v8)
{
    return {(v1[0]+v2[0]+v3[0]+v4[0]+v5[0]+v6[0]+v7[0]+v8[0])/8.f, (v1[1]+v2[1]+v3[1]+v4[1]+v5[1]+v6[1]+v7[1]+v8[1])/8.f, (v1[2]+v2[2]+v3[2]+v4[2]+v5[2]+v6[2]+v7[2]+v8[2])/8.f};
}



/**
 * Returns the normal vector for the terrain vertex at the given row and column.
 */
glm::vec3 Terrain::getNormal(int row, int col) {
    // TODO: Compute the normal at the given row and column using the positions of the
    //       neighboring vertices.

    glm::vec3 center = getPosition(row, col);

    glm::vec3 one   = center - getPosition(row-1, col-1);
    glm::vec3 two   = center - getPosition(row-1, col  );
    glm::vec3 three = center - getPosition(row-1, col+1);
    glm::vec3 four  = center - getPosition(row  , col-1);
    glm::vec3 five  = center - getPosition(row  , col+1);
    glm::vec3 six   = center - getPosition(row+1, col-1);
    glm::vec3 seven = center - getPosition(row+1, col  );
    glm::vec3 eight = center - getPosition(row+1, col+1);


    glm::vec3 oneP   = glm::cross(one  , two  );
    glm::vec3 twoP   = glm::cross(two  , three);
    glm::vec3 threeP = glm::cross(three, five );
    glm::vec3 fourP  = glm::cross(five , eight);
    glm::vec3 fiveP  = glm::cross(eight, seven);
    glm::vec3 sixP   = glm::cross(seven, six  );
    glm::vec3 sevenP = glm::cross(six  , four );
    glm::vec3 eightP = glm::cross(four , one  );


    return glm::normalize(average8Vectors(oneP, twoP, threeP, fourP, fiveP, sixP, sevenP, eightP));

    //return glm::vec3(0, 1, 0);
}


/**
 * Initializes the terrain by storing positions and normals in a vertex buffer.
 */
void Terrain::init() {
    // TODO: Change from GL_LINE to GL_FILL in order to render full triangles instead of wireframe.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    // Initializes a grid of vertices using triangle strips.
    int numVertices = (m_numRows - 1) * (2 * m_numCols + 2);
    std::vector<glm::vec3> data(2 * numVertices);
    int index = 0;
    for (int row = 0; row < m_numRows - 1; row++) {
        for (int col = m_numCols - 1; col >= 0; col--) {
            data[index++] = getPosition(row, col);
            data[index++] = getNormal  (row, col);
            data[index++] = getPosition(row + 1, col);
            data[index++] = getNormal  (row + 1, col);
        }
        data[index++] = getPosition(row + 1, 0);
        data[index++] = getNormal  (row + 1, 0);
        data[index++] = getPosition(row + 1, m_numCols - 1);
        data[index++] = getNormal  (row + 1, m_numCols - 1);
    }

    // Initialize OpenGLShape.
    m_shape = std::make_unique<OpenGLShape>();
    m_shape->setVertexData(&data[0][0], data.size() * 3, VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, numVertices);
    m_shape->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_shape->setAttribute(ShaderAttrib::NORMAL, 3, sizeof(glm::vec3), VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_shape->buildVAO();
}


/**
 * Draws the terrain.
 */
void Terrain::draw()
{
    m_shape->draw();
}
