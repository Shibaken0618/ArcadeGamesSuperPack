#include "block.hpp"

Block::Block(Vector2 position) {
    this -> position = position;
}

void Block::Draw() {
    DrawRectangle(position.x, position.y, 4, 4, YELLOW);
}

Rectangle Block::getRect() {
    Rectangle rect;
    rect.x = position.x;
    rect.y = position.y;
    rect.width = 4;
    rect.height = 4;
    return rect;
}