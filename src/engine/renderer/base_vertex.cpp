#include "base_vertex.h"

bool Vertex::operator==(const Vertex &other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}
