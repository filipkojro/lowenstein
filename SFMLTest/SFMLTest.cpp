#include <iostream>
#include <cmath>
#include <cstdio>
#include <SFML/Graphics.hpp>

#define PI 3.14159265358979323846


const int screenWidth = 640;
const int screenHeight = 480;
const int FOV = 80;

void clearBuffer(sf::Image* img) {
    for (int x = 0; x < screenWidth; x++) {
        for (int y = 0; y < screenHeight; y++) {
            img->setPixel(x, y, sf::Color::Transparent);
        }
    }
}

void drawLine(int x1, int y1, int x2, int y2, sf::Color color, sf::Image* img) {

    float hx = x1 > x2 ? x1 : x2;
    float lx = x1 < x2 ? x1 : x2;
    float hy = y1 > y2 ? y1 : y2;
    float ly = y1 < y2 ? y1 : y2;

    if (x1 == x2) {
        //std::cout << "why 0\n";

        for (int y = ly; y <= hy; y++) {
            if (y >= 0 && y < screenHeight) {
                img->setPixel(x1, y, color);
            }
        }
    }
    else {
        float a = (float(y2) - float(y1)) / (float(x2) - float(x1));
        float b = y1 - (a * x1);
        int x;
        int y;

        //std::cout << a << "\n";

        if (abs(a) > 1) {
            for (y = ly; y <= hy; y++) {
                x = (y - b) / a;
                if (y >= 0 && y < screenHeight && x >= 0 && x < screenWidth) {
                    img->setPixel(x, y, color);
                }
            }
        }
        else {
            for (x = lx; x <= hx; x++) {
                y = (a * x) + b;
                if (y >= 0 && y < screenHeight && x >= 0 && x < screenWidth) {
                    img->setPixel(x, y, color);
                }
            }
        }
    }
}

void generateDistanceMap(float* distanceMap, float* wallsTab, float* addWallInfo, int wallCount, float sourceX, float sourceY, float sourceRotation) {

    for (int sx = 0; sx < screenWidth; sx++) {

        float beta = sourceRotation + (FOV / 2) - ((sx * FOV) / screenWidth);

        //beta = beta * PI / 180;
        
        if (beta == 90 || beta == 270) {
            beta = (beta - 1) * PI / 180;
        }
        else {
            beta = beta * PI / 180;
        }
        float a = tan(beta);

        float b = sourceY - (a * sourceX);

        distanceMap[sx] = 600;//WARNING temporary const

        for (int currentWall = 0; currentWall < wallCount; currentWall++) {
            if (a != addWallInfo[currentWall * 2]) {
                float collisionX = (addWallInfo[currentWall * 2 + 1] - b) / (a - addWallInfo[currentWall * 2]);
                float collisionY = a * collisionX + b;

                float distance = sqrt(pow(sourceX - collisionX, 2) + pow(sourceY - collisionY, 2));
                if (distance < distanceMap[sx])distanceMap[sx] = distance;

                //higher x
                float hx = wallsTab[currentWall * 4] > wallsTab[currentWall * 4 + 2] ? wallsTab[currentWall * 4] : wallsTab[currentWall * 4 + 2];
                float lx = wallsTab[currentWall * 4] < wallsTab[currentWall * 4 + 2] ? wallsTab[currentWall * 4] : wallsTab[currentWall * 4 + 2];
                if (collisionX < lx || hx < collisionX)distanceMap[sx] = 0;

                //higher y
                float hy = wallsTab[currentWall * 4 + 1] > wallsTab[currentWall * 4 + 3] ? wallsTab[currentWall * 4 + 1] : wallsTab[currentWall * 4 + 3];
                float ly = wallsTab[currentWall * 4 + 1] < wallsTab[currentWall * 4 + 3] ? wallsTab[currentWall * 4 + 1] : wallsTab[currentWall * 4 + 3];
                /*
                if (collisionY < ly || hy < collisionY) {
                    distanceMap[sx] = 0;
                    std::cout << collisionY << "\n";
                }*/
            }
        }
    }
}

int overFlowInt(float num, int min, int max) {
    if (num > max)return max;
    if (num < min)return min;
    return int(num);
}

void imageFromDistacneMap(sf::Image* buffer, float* distanceMap, int scale, sf::Color color) {
    for (int x = 0; x < screenWidth; x++) {
        for (int y = 0; y < overFlowInt(distanceMap[x] * scale / 2, 0, screenHeight / 2) - 1; y++) {
            //buffer->setPixel(1, 1, color);
            buffer->setPixel(x, overFlowInt((screenHeight / 2) + y, 0, screenHeight), color);
            buffer->setPixel(x, overFlowInt((screenHeight / 2) - y, 0, screenHeight), color);
        }
    }
}

int main(){
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Lowenstein");

    sf::Image buffer;
    buffer.create(screenWidth, screenHeight, sf::Color::Black);

    sf::Texture bufferTexture;
    bufferTexture.create(window.getSize().x, window.getSize().y);
    bufferTexture.update(buffer);

    sf::Sprite bufferSprite;
    bufferSprite.setTexture(bufferTexture);


    constexpr int wallCount = 1;
    float wallsTab[wallCount * 4] = {
        0, 0, 10, 5
    };

    //two dimensional array [a, b]
    float addWallInfo[wallCount * 2] = {0, 0};

    float playerPosX = 5, playerPosY = -5, playerRotation = 1;

    float distanceMap[screenWidth];

    generateDistanceMap(distanceMap, wallsTab, addWallInfo, wallCount, playerPosX, playerPosY, playerRotation);

    for (int i = 0; i < screenWidth; i++) {
        std::cout << distanceMap[i] << "\n";
    }

    while (window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }

        clearBuffer(&buffer);
        //playerRotation = 96;
        
        playerRotation++;
        if (playerRotation == 360)playerRotation = 0;
        //std::cout << playerRotation;
        
        
        generateDistanceMap(distanceMap, wallsTab, addWallInfo, wallCount, playerPosX, playerPosY, playerRotation);
        
        imageFromDistacneMap(&buffer, distanceMap, 1, sf::Color::White);
        /*
        for (int i = 0; i < screenWidth; i++) {
            //if (i == 0)std::cout << " " << distanceMap[i] << "\n";
            if(distanceMap[i] > 640)drawLine(i, 0, i, 0, sf::Color::White, &buffer);
            else drawLine(i, 0, i, distanceMap[i] * 20, sf::Color::White, &buffer);
            
        }
        */
        //drawLine(50, 50, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, sf::Color::White, &buffer);
        //std::cout << sf::Mouse::getPosition(window).x << " " << sf::Mouse::getPosition(window).y << "\n";

        





        window.clear();
        bufferTexture.update(buffer);
        window.draw(bufferSprite);
        window.display();
    }

    return 0;
}