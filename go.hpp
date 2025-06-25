#ifndef SPEEDGO_HPP
#define SPEEDGO_HPP
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <list>
#include <unordered_set>

#define INTERSECTION_SIZE 50
#define BOARD_W 19
#define BOARD_H 19

#define COLOR_BG (sf::Color(177, 157, 127))
#define COLOR_INTERSECTION (sf::Color(167, 147, 117))
#define COLOR_GAME_OVER (sf::Color(177, 97, 57))
#define COLOR_GAME_OVER_INTERSECTION (sf::Color(157, 77, 37))

class Go {
public:
    enum IntersectionType {
        Free,
        Black,
        White,
        Beyond
    };

    enum Direction {
        Up,
        Right,
        Down,
        Left
    };

    explicit Go(bool white = false) : m_white(white) {
        for (std::size_t i = 0; i < BOARD_H; ++i) {
            for (std::size_t j = 0; j < BOARD_W; ++j) {
                m_itrsns[i][j] = Free;
            }
        }
    }

    bool
    wouldBeCaptured(const sf::Vector2i &itrsn) {
        assert(getIntersection(itrsn) != Beyond);
        std::unordered_set<std::size_t> itrsns =
            getCapturedStones(itrsn, who());
        IntersectionType old = m_itrsns[itrsn.y][itrsn.x];
        m_itrsns[itrsn.y][itrsn.x] = who();

        for (std::size_t c : itrsns) {
            IntersectionType adj[4];
            getAdjacent(idx2itrsn(c), adj);
            if (std::any_of(adj, adj + 4, [](IntersectionType t) {
                    return !canOccupy(t);
                })) {
                m_itrsns[itrsn.y][itrsn.x] = old;
                return false;
            }
        }

        m_itrsns[itrsn.y][itrsn.x] = old;
        return true;
    }

    bool
    wouldCapture(const sf::Vector2i &itrsn) {
        assert(getIntersection(itrsn) != Beyond);
        IntersectionType old = m_itrsns[itrsn.y][itrsn.x];
        m_itrsns[itrsn.y][itrsn.x] = who();
        m_white = !m_white;
        Direction dir = FirstDirection;
        bool yes = false;
        do {
            if (getIntersection(adjacent(itrsn, dir)) != who()) {
                continue;
            }
            yes = wouldBeCaptured(adjacent(itrsn, dir));
            if (yes) {
                break;
            }
        } while (nextDirection(dir));
        m_white = !m_white;
        m_itrsns[itrsn.y][itrsn.x] = old;
        return yes;
    }

    sf::Vector2i
    pickFreeIntersection() {
        std::size_t nrLegalIntersections = 0;
        for (std::size_t i = 0; i < BOARD_H; ++i) {
            for (std::size_t j = 0; j < BOARD_W; ++j) {
                m_itrsnFlagCache[i][j] = false;
                sf::Vector2i itrsn(j, i);
                if (m_itrsns[i][j] != Free) {

                } else if (!wouldBeCaptured(itrsn) || wouldCapture(itrsn)) {
                    ++nrLegalIntersections;
                    m_itrsnFlagCache[i][j] = true;
                }
            }
        }

        if (nrLegalIntersections == 0) {
            return sf::Vector2i(BOARD_W, BOARD_H);
        }

        std::size_t r = rand();
        r %= nrLegalIntersections;
        r += 1;

        for (std::size_t i = 0; i < BOARD_H; ++i) {
            for (std::size_t j = 0; j < BOARD_W; ++j) {
                if (!m_itrsnFlagCache[i][j]) {
                    continue;
                }
                --r;
                if (r == 0) {
                    return sf::Vector2i(j, i);
                }
            }
        }

        return sf::Vector2i(BOARD_W, BOARD_H);
    }

    std::size_t
    putStone(const sf::Vector2i &itrsn) {
        assert(m_itrsns[itrsn.y][itrsn.x] == Free);
        m_itrsns[itrsn.y][itrsn.x] = who();
        std::size_t pris = checkPrisoners(itrsn);
        m_white = !m_white;
        return pris;
    }

    IntersectionType
    who() {
        return m_white ? White : Black;
    }

    IntersectionType
    enemy() {
        return m_white ? Black : White;
    }

    IntersectionType
    getIntersection(const sf::Vector2i &itrsn) {
        if (itrsn.x < 0 || itrsn.y < 0 || itrsn.x >= BOARD_W ||
            itrsn.y >= BOARD_H) {
            return Beyond;
        }
        return m_itrsns[itrsn.y][itrsn.x];
    }

    std::size_t
    getNr(bool white) {
        std::size_t nr = 0;
        for (std::size_t i = 0; i < BOARD_H; ++i) {
            for (std::size_t j = 0; j < BOARD_W; ++j) {
                if ((white && m_itrsns[i][j] == White) ||
                    (!white && m_itrsns[i][j] == Black)) {
                    ++nr;
                }
            }
        }
        return nr;
    }

    void
    switchPlayer() {
        m_white = !m_white;
    }

private:
    static bool
    isOccupiedByAny(IntersectionType type) {
        return type == Black || type == White;
    }

    static bool
    canOccupy(IntersectionType type) {
        return type != Free;
    }

    // do while
    static const Direction FirstDirection = Up;
    static bool
    nextDirection(Direction &dir) {
        switch (dir) {
        case Up:
            dir = Right;
            return true;
        case Right:
            dir = Down;
            return true;
        case Down:
            dir = Left;
            return true;
        case Left:
        default:
            break;
        }
        return false;
    }

    static sf::Vector2i
    adjacent(const sf::Vector2i &isn, Direction dir) {
        switch (dir) {
        case Up:
            return sf::Vector2i(isn.x, isn.y - 1);
        case Down:
            return sf::Vector2i(isn.x, isn.y + 1);
        case Right:
            return sf::Vector2i(isn.x + 1, isn.y);
        case Left:
            return sf::Vector2i(isn.x - 1, isn.y);
        default:
            break;
        }
        return isn;
    }

    static std::size_t
    itrsn2idx(const sf::Vector2i &itrsn) {
        return itrsn.x + (std::size_t)itrsn.y * BOARD_W;
    }

    static sf::Vector2i
    idx2itrsn(const std::size_t idx) {
        return sf::Vector2i(idx % BOARD_W, idx / BOARD_W);
    }

    void
    getAdjacent(const sf::Vector2i &isn, IntersectionType *adj) {
        adj[Up] = getIntersection(sf::Vector2i(isn.x, isn.y - 1));
        adj[Down] = getIntersection(sf::Vector2i(isn.x, isn.y + 1));
        adj[Left] = getIntersection(sf::Vector2i(isn.x - 1, isn.y));
        adj[Right] = getIntersection(sf::Vector2i(isn.x + 1, isn.y));
    }

    std::unordered_set<std::size_t>
    getCapturedStones(const sf::Vector2i &itrsn, IntersectionType whose) {
        assert(getIntersection(itrsn) != Beyond);
        IntersectionType old = m_itrsns[itrsn.y][itrsn.x];
        m_itrsns[itrsn.y][itrsn.x] = whose;
        std::unordered_set<std::size_t> itrsns;
        itrsns.insert(itrsn2idx(itrsn));
        std::list<std::size_t> toCheck;
        Direction dir = FirstDirection;
        do {
            if (getIntersection(adjacent(itrsn, dir)) != Beyond) {
                sf::Vector2i adj = adjacent(itrsn, dir);
                toCheck.push_back(itrsn2idx(adj));
            }
        } while (nextDirection(dir));

        while (!toCheck.empty()) {
            sf::Vector2i itrsn = idx2itrsn(toCheck.front());
            toCheck.pop_front();
            if (itrsns.find(itrsn2idx(itrsn)) != itrsns.end()) {
                continue;
            }
            if (getIntersection(itrsn) == whose) {
                itrsns.insert(itrsn2idx(itrsn));
                Direction dir = FirstDirection;
                do {
                    if (getIntersection(adjacent(itrsn, dir)) != Beyond) {
                        toCheck.push_back(itrsn2idx(adjacent(itrsn, dir)));
                    }
                } while (nextDirection(dir));
            }
        }

        m_itrsns[itrsn.y][itrsn.x] = old;
        return itrsns;
    }

    std::size_t
    checkPrisoners(const sf::Vector2i &lastIntersection) {
        Direction dir = FirstDirection;
        std::unordered_set<std::size_t> captured;
        do {
            if (getIntersection(adjacent(lastIntersection, dir)) != enemy()) {
                continue;
            }
            m_white = !m_white;
            if (!wouldBeCaptured(adjacent(lastIntersection, dir))) {
                m_white = !m_white;
                continue;
            }
            m_white = !m_white;
            captured.merge(
                getCapturedStones(adjacent(lastIntersection, dir), enemy()));
        } while (nextDirection(dir));

        for (std::size_t c : captured) {
            m_itrsns[idx2itrsn(c).y][idx2itrsn(c).x] = Free;
        }

        return captured.size();
    }

    bool m_white;
    IntersectionType m_itrsns[BOARD_H][BOARD_W];
    bool m_itrsnFlagCache[BOARD_H][BOARD_W];
};

#endif // SPEEDGO_HPP
