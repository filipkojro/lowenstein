#include <iostream>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>

#define PI 3.14159265358979323846
#define floatRounfingPrecision 10000

const int screenWidth = 640;
const int screenHeight = 480;
const int FOV = 80;

float rounding(float x) {
    x *= floatRounfingPrecision;
    x = round(x);
    x /= floatRounfingPrecision;
    return x;
}

float mapping(float input, float inMin, float inMax, float outMin, float outMax) {
    float difIn = inMax - inMin;
    float difOut = outMax - outMin;

    return input * difOut / difIn;
}

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

void generateDistanceMap(int thID, int thSize, float* distanceMap, float* wallsTab, float* addWallInfo, int wallCount, float sourceX, float sourceY, float sourceRotation) {
    for (int sx = thID * thSize; sx < thID * thSize + thSize; sx++) {

        float beta = sourceRotation + (FOV / 2) - float((float(sx * FOV)) / screenWidth);//to psulo dokladnosc :(
        float betaR;

        /*
        if (beta == 90 || beta == 270) {
            betaR = (beta - 1) * PI / 180;
        }
        else {
            betaR = beta * PI / 180;
        }
        */
        betaR = beta * PI / 180;

        //float a = tan(betaR);
        float a = sin(betaR) / cos(betaR);//possible optimalization (internet)
        float b = sourceY - (a * sourceX);

        distanceMap[sx] = INFINITY;//WARNING temporary const

        for (int currentWall = 0; currentWall < wallCount; currentWall++) {
            if (a != addWallInfo[currentWall * 2]) {
                float wallA = addWallInfo[currentWall * 2];
                float wallB = addWallInfo[currentWall * 2 + 1];

                float collisionX = (wallB - b) / (a - wallA);
                float collisionY = a * collisionX + b;

                if (wallA == INFINITY || wallA == -INFINITY) {
                    collisionX = wallsTab[currentWall * 4];
                    collisionY = a * collisionX + b;
                }

                
                
                //float TcollisionX = cos(beta) * distance;
                //float TcollisionY = sin(beta) * distance;

                //std::cout << "N " << collisionX << " : " << collisionY << "\n";
                //std::cout << "T " << TcollisionX << " : " << TcollisionY << "\n\n";

                //if (collisionX * TcollisionX  < 0)continue;
                //if (collisionY * TcollisionY < 0)continue;

                //std::cout << collisionX << " : " << collisionY << "\n";

                if (0 <= beta && beta < 90)if (collisionX < sourceX || collisionY < sourceY)continue;
                if (90 <= beta && beta < 180)if (collisionX > sourceX || collisionY < sourceY)continue;
                if (180 <= beta && beta < 270)if (collisionX > sourceX || collisionY > sourceY)continue;
                if (270 <= beta && beta < 360)if (collisionX < sourceX || collisionY > sourceY)continue;
                
                //higher x
                float hx = wallsTab[currentWall * 4] > wallsTab[currentWall * 4 + 2] ? wallsTab[currentWall * 4] : wallsTab[currentWall * 4 + 2];
                float lx = wallsTab[currentWall * 4] < wallsTab[currentWall * 4 + 2] ? wallsTab[currentWall * 4] : wallsTab[currentWall * 4 + 2];
                if (rounding(collisionX) < rounding(lx) || rounding(hx) < rounding(collisionX)){
                        //distanceMap[sx] = INFINITY;
                    continue;
                }

                //higher y
                float hy = wallsTab[currentWall * 4 + 1] > wallsTab[currentWall * 4 + 3] ? wallsTab[currentWall * 4 + 1] : wallsTab[currentWall * 4 + 3];
                float ly = wallsTab[currentWall * 4 + 1] < wallsTab[currentWall * 4 + 3] ? wallsTab[currentWall * 4 + 1] : wallsTab[currentWall * 4 + 3];
                if (rounding(collisionY) < rounding(ly) || rounding(hy) < rounding(collisionY)) {
                    //distanceMap[sx] = INFINITY;
                    //std::cout << collisionY << "\n";
                    continue;
                }

                //float distance = sqrt(pow(sourceX - collisionX, 2) + pow(sourceY - collisionY, 2));

                float distance = pow(sourceX - collisionX, 2) + pow(sourceY - collisionY, 2);

                if (distance < distanceMap[sx])distanceMap[sx] = distance;
            }
        }
    }
}

void generateDistanceMap2(float* distanceMap, float* wallsTab, int wallCount, float sourceX, float sourceY, float sourceRotation) {
    float renderDistance = 500;

    for (int sx = 0; sx < screenWidth; sx++) {

        float beta = sourceRotation + (FOV / 2) - float((float(sx * FOV)) / screenWidth);//to psulo dokladnosc :(
        float betaR;

        betaR = beta * PI / 180;

        float endOfRenderX = cos(betaR) * renderDistance + sourceX;//sin or cos idk???
        float endOfRenderY = sin(betaR) * renderDistance + sourceY;

        distanceMap[sx] = INFINITY;

        for (int curWall = 0; curWall < wallCount; curWall++) {

            float x3 = wallsTab[curWall * 4];
            float y3 = wallsTab[curWall * 4 + 1];
            float x4 = wallsTab[curWall * 4 + 2];
            float y4 = wallsTab[curWall * 4 + 3];

            //https://jeffreythompson.org/collision-detection/line-line.php <3

            float uA = ((x4 - x3) * (sourceY - y3) - (y4 - y3) * (sourceX - x3))
                    / ((y4 - y3) * (endOfRenderX - sourceX) - (x4 - x3) * (endOfRenderY - sourceY));

            float uB = ((endOfRenderX - sourceX) * (sourceY - y3) - (endOfRenderY - sourceY) * (sourceX - x3))
                    / ((y4 - y3) * (endOfRenderX - sourceX) - (x4 - x3) * (endOfRenderY - sourceY));

            if (uA < 0 || uA > 1 || uB < 0 || uB > 1) {
                continue;
            }

            float collisionX = sourceX + (uA * (endOfRenderX - sourceX));
            float collisionY = sourceY + (uA * (endOfRenderY - sourceY));

            float distance = pow(sourceX - collisionX, 2) + pow(sourceY - collisionY, 2);

            if (distance < distanceMap[sx]) distanceMap[sx] = distance;
        }
    }
}

void generateAddWallInfo(int wallCount, float* wallsTab, float* addWallInfo) {

    int ac = 0;
    int wc = 0;
    for (int i = 0; i < wallCount; i++) {
        addWallInfo[ac] = (wallsTab[wc + 1] - wallsTab[wc + 3]) / (wallsTab[wc] - wallsTab[wc + 2]);
        addWallInfo[ac + 1] = wallsTab[wc + 1] - (addWallInfo[ac] * wallsTab[wc]);

        ac += 2;
        wc += 4;
    }
}

int overFlowInt(float num, int min, int max) {
    if (num > max)return max;
    if (num < min)return min;
    return int(num);
}

void makeMap(sf::Image* mapBuffer, float* wallsTab, int wallCount, float playerX, float playerY, float playerRotation) {
    
    //player vision cone
    int lenght = sqrt(pow(screenWidth / 2, 2) + pow(screenHeight / 2, 2));

    for (int i = 0; i < FOV; i++) {
        double deg = (i + playerRotation - (FOV / 2)) * PI / 180;

        int x = round(cos(deg) * lenght);
        int y = round(sin(deg) * lenght);

        //std::cout << x << " " << y << " " << playerX << " " << playerY << "\n";

        drawLine(screenWidth / 2, screenHeight / 2, x + (screenWidth / 2), y + (screenHeight / 2), sf::Color(0,0,255, 100), mapBuffer);
    }
    
    
    //drawing walls
    for (int i = 0; i < wallCount; i++) {
        drawLine(wallsTab[(i * 4)] + (screenWidth / 2) - playerX, wallsTab[(i * 4) + 1] + (screenHeight / 2) - playerY, wallsTab[(i * 4) + 2] + (screenWidth / 2) - playerX, wallsTab[(i * 4) + 3] + (screenHeight / 2) - playerY, sf::Color::White, mapBuffer);
    }
}

void imageFromDistacneMap(sf::Image* buffer, float* distanceMap, int scale, sf::Color color) {
    for (int i = 0; i < screenWidth; i++) {
        float distance = sqrt(distanceMap[i]);
        sf::Color col = sf::Color(color.r, color.g, color.b, mapping(distance, 0, screenHeight / 3, 255, 0));
        if (distance < screenHeight / 2) {
            drawLine(i, screenHeight / 2, i, (distance), col, buffer);
            drawLine(i, screenHeight / 2, i, (screenHeight / 2) + (screenHeight / 2) - (distance), col, buffer);
        }
        else {
            drawLine(i, screenHeight / 2, i, screenHeight / 2, col, buffer);
        }
        //std::cout << distanceMap[i] << "\n";
    }
}

void playerMovement(float* playerX, float* playerY, float* playerR, double lastT) {
    float speed = 0.5f;

    //vector of next move
    float vectorx = 0;
    float vectory = 0;
    //which direction will move
    float moveR = 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && sf::Keyboard::isKeyPressed(sf::Keyboard::W))(*playerR)--;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))(*playerR)++;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))(*playerX)--;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))(*playerX)++;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))(*playerY)++;//reversed
    //if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) 

    if (*playerR >= 360)*playerR = 0;
    else if (*playerR < 0)*playerR = 359;

    //*playerX = float(cos(*playerR * PI / 180) * -60);
    //*playerY = float(sin(*playerR * PI / 180) * -60);
}

int main(){

    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Lowenstein Game");
    sf::RenderWindow mapWindow(sf::VideoMode(screenWidth, screenHeight), "Lowenstein Map");

    sf::Image buffer;
    buffer.create(screenWidth, screenHeight, sf::Color::Black);
    sf::Image mapBuffer;
    mapBuffer.create(screenWidth, screenHeight, sf::Color::Black);

    sf::Texture bufferTexture;
    bufferTexture.create(window.getSize().x, window.getSize().y);
    bufferTexture.update(buffer);
    sf::Texture mapBufferTexture;
    mapBufferTexture.create(mapWindow.getSize().x, mapWindow.getSize().y);
    mapBufferTexture.update(mapBuffer);

    sf::Sprite bufferSprite;
    bufferSprite.setTexture(bufferTexture);
    sf::Sprite mapBufferSprite;
    mapBufferSprite.setTexture(mapBufferTexture);


    //pulling map from file
    //wall count
    //x1.1 y1.1 x1.2 y1.2
    //x2.1 y2.1 x2.2 y2.2 ...
    
    //std::ifstream in("rndm.map");
    std::ifstream in("level0.map");

    int wallCount;
    in >> wallCount;

    float* wallsTab = new float[wallCount * 4];
    for (int i = 0; i < wallCount * 4; i++) {
        in >> wallsTab[i];
    }

    in.close();

    //two dimensional array [a, b]
    float* addWallInfo = new float[wallCount * 2];

    generateAddWallInfo(wallCount, wallsTab, addWallInfo);


    for (int i = 0; i < wallCount * 2; i++) {
        std::cout << addWallInfo[i] << "\n";
    }

    float playerPosX = -15, playerPosY = -30, playerRotation = 96;

    auto lastTime = std::chrono::steady_clock::now();

    float distanceMap[screenWidth];

    //generateDistanceMap(0, screenWidth, distanceMap, wallsTab, addWallInfo, wallCount, playerPosX, playerPosY, playerRotation);
    generateDistanceMap2(distanceMap, wallsTab, wallCount, playerPosX, playerPosY, playerRotation);


    //main game loop!!!

    while (window.isOpen() && mapWindow.isOpen()){

        //wnidow close check
        sf::Event event;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }
        while (mapWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                mapWindow.close();
        }
        
        clearBuffer(&buffer);
        clearBuffer(&mapBuffer);
        

        //controls
        playerMovement(&playerPosX, &playerPosY, &playerRotation, 13);

        std::chrono::duration<double> elps = std::chrono::steady_clock::now() - lastTime;
        lastTime = std::chrono::steady_clock::now();

        std::cout << elps.count() << "\n";

        //rendering walls
        //generateDistanceMap(0, screenWidth, distanceMap, wallsTab, addWallInfo, wallCount, playerPosX, playerPosY, playerRotation);
        generateDistanceMap2(distanceMap, wallsTab, wallCount, playerPosX, playerPosY, playerRotation);
        
        imageFromDistacneMap(&buffer, distanceMap, 1, sf::Color::White);



        //generating map
        makeMap(&mapBuffer, wallsTab, wallCount, playerPosX, playerPosY, playerRotation);



        //display part
        window.clear();
        bufferTexture.update(buffer);
        window.draw(bufferSprite);
        window.display();

        mapWindow.clear();
        mapBufferTexture.update(mapBuffer);
        mapWindow.draw(mapBufferSprite);
        mapWindow.display();
    }

    return 0;
}