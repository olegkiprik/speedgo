#include "go.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <cstdlib>
#include <iostream>

int
main() {
    std::srand(std::time(0));

    sf::RenderWindow window(
        sf::VideoMode(BOARD_W * INTERSECTION_SIZE, BOARD_H * INTERSECTION_SIZE),
        "SpeedGo");
    window.setVerticalSyncEnabled(true);

    sf::RectangleShape bg;
    bg.setSize(
        sf::Vector2f(INTERSECTION_SIZE * BOARD_W, INTERSECTION_SIZE * BOARD_H));
    bg.setFillColor(COLOR_BG);

    sf::CircleShape stones[BOARD_H][BOARD_W];
    for (int i = 0; i < BOARD_H; ++i) {
        for (int j = 0; j < BOARD_W; ++j) {
            stones[i][j].setFillColor(COLOR_INTERSECTION);
            stones[i][j].setRadius(INTERSECTION_SIZE / 2.f);
            stones[i][j].setPosition(j * INTERSECTION_SIZE,
                                     i * INTERSECTION_SIZE);
        }
    }

    Go go;

    std::size_t whitePris = 0;
    std::size_t nrStep = 0;
    sf::Clock cl;

    bool gameOver = false;

    while (window.isOpen()) {
        sf::Event e;
        if (window.waitEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonPressed: {
                if (gameOver || go.who() == Go::White) {
                    break;
                }

                sf::Vector2i coords(e.mouseButton.x, e.mouseButton.y);
                sf::Vector2i itrsn(coords.x / INTERSECTION_SIZE,
                                   coords.y / INTERSECTION_SIZE);

                if (go.getIntersection(itrsn) != Go::Free ||
                    (go.wouldBeCaptured(itrsn) && !go.wouldCapture(itrsn))) {
                    break;
                }

                std::size_t pris = go.putStone(itrsn);

                for (std::size_t i = 0; i < BOARD_H; ++i) {
                    for (std::size_t j = 0; j < BOARD_W; ++j) {
                        if (go.getIntersection(sf::Vector2i(j, i)) ==
                                Go::Free &&
                            go.wouldCapture(sf::Vector2i(j, i))) {
                            gameOver = true;
                            break;
                        }
                    }
                }

                whitePris += pris;
                ++nrStep;
                break;
            }
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Escape) {
                    if (gameOver) {
                        window.close();
                    } else {
                        gameOver = true;
                    }
                }
                break;
            default:
                break;
            }
        }

        if (go.who() == Go::White) {
            sf::Vector2i enemyIntersection = go.pickFreeIntersection();
            if (enemyIntersection != sf::Vector2i(BOARD_W, BOARD_H)) {
                std::size_t pris = go.putStone(enemyIntersection);
                if (pris != 0) {
                    gameOver = true;
                }
            } else {
                if (go.getNr(Go::White == Go::White) == 0) {
                    std::cout << "YOU WIN!" << std::endl;
                } else {
                    gameOver = true;
                    std::cout << "YOU LOSE!" << std::endl;
                }
                std::cout << "Steps: " << nrStep << std::endl;
                std::cout << "Time: " << cl.getElapsedTime().asSeconds() << " s"
                          << std::endl;
                go.switchPlayer();
            }
        }

        for (int i = 0; i < BOARD_H; ++i) {
            for (int j = 0; j < BOARD_W; ++j) {
                if (go.getIntersection(sf::Vector2i(j, i)) == Go::Free) {
                    stones[i][j].setFillColor(gameOver
                                                  ? COLOR_GAME_OVER_INTERSECTION
                                                  : COLOR_INTERSECTION);
                } else if (go.getIntersection(sf::Vector2i(j, i)) ==
                           Go::White) {
                    stones[i][j].setFillColor(sf::Color::White);
                } else {
                    stones[i][j].setFillColor(sf::Color::Black);
                }
            }
        }

        bg.setFillColor(gameOver ? COLOR_GAME_OVER : COLOR_BG);

        window.clear();
        window.draw(bg);

        for (int i = 0; i < BOARD_H; ++i) {
            for (int j = 0; j < BOARD_W; ++j) {
                window.draw(stones[i][j]);
            }
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
