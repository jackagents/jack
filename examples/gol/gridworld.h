// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#pragma once

#include <jack/jack.h>
#include <SFML/Graphics.hpp>

/*! ***********************************************************************************************
 * \class GridWorld
 *
 * Encapsulates a SFML window that is capable of supplying coordinates to a 2D
 * grid coordinate in screen space.
 * ************************************************************************************************/
namespace aos
{
struct GridWorld {
    GridWorld(aos::jack::Engine &engine,
              int gridSizeX,
              int gridSizeY,
              int windowWidth,
              int windowHeight,
              std::string &&title,
              unsigned int frameRate = 60,
              float gridScale        = 1.f)
    : m_engine(engine)
    , m_gridSizeX(std::max(gridSizeX, 1))
    , m_gridSizeY(std::max(gridSizeY, 1))
    , m_gridScale(gridScale)
    , m_window(sf::VideoMode(windowWidth, windowHeight), std::move(title))
    , m_frameRate(frameRate)
    {
        m_window.setFramerateLimit(frameRate);
    }

    /**
     * @brief Given a 2d coordinate, return the world space rect, representing the 2d grid
     * coordinate.
     * @param x The x grid coordinate. This parameter is clamped to the bounds of the window.
     * @param y The y grid coordinate. This parameter is clamped to the bounds of the window.
     **/
    sf::Rect<float> cellAsRect(float x, float y) const {
        x = std::max(x, 0.f);
        x = std::min(x, static_cast<float>(m_gridSizeX - 1));

        y = std::max(y, 0.f);
        y = std::min(y, static_cast<float>(m_gridSizeY - 1));

        sf::Vector2f    cellSize = gridCellSize();
        sf::Rect<float> result(m_gridStartOffset.x + (cellSize.x * x),
                               m_gridStartOffset.y + (cellSize.y * y),
                               cellSize.x,
                               cellSize.y);
        return result;
    }

    /**
     * @brief Given a 2d coordinate, return the screen space rect, representing the 2d grid
     * coordinate.
     * @param x The x grid coordinate. This parameter is clamped to the bounds of the window.
     * @param y The y grid coordinate. This parameter is clamped to the bounds of the window.
     **/
    sf::Rect<float> cellAsRectScreenSpace(float x, float y) const {
        sf::Rect<float> cell   = cellAsRect(x, y);
        sf::Vector2i    startP = m_window.mapCoordsToPixel(sf::Vector2f(cell.left, cell.top));
        sf::Vector2i    endP =
            m_window.mapCoordsToPixel(sf::Vector2f(cell.left + cell.width, cell.top + cell.height));

        sf::Rect<float> result;
        result.left   = static_cast<float>(startP.x);
        result.top    = static_cast<float>(startP.y);
        result.width  = static_cast<float>(endP.x - startP.x);
        result.height = static_cast<float>(endP.y - startP.y);
        return result;
    }

    /**
     * @brief Draw the screen space rectangular cell at coordinates x, y
     * @param x The x grid coordinate to draw. This parameter is clamped to the bounds of the window.
     * @param y The y grid coordinate to draw. This parameter is clamped to the bounds of the window.
     * @param color The color of the rectangle to draw
     **/
    void drawCell(int x, int y, const sf::Color &color) {
        sf::Rect<float> cell = cellAsRect(static_cast<float>(x), static_cast<float>(y));
        sf::RectangleShape rectangle;
        rectangle.setSize(sf::Vector2f(cell.width, cell.height));
        rectangle.setPosition(sf::Vector2f(cell.left, cell.top));
        rectangle.setFillColor(color);
        m_window.draw(rectangle);
    }

    /**
     * @brief Ticks the engine and draws the world. It should be called once per
     * rendering loop where all previously drawn commands since the last draw()
     * command was issued is displayed on screen.
     **/
    bool draw(int engineTicks, float deltaTimeS) {
        m_window.display();
        m_window.clear();
        bool result     = true;
        sf::Event event = {};
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
                result = false;
            } else if (event.type == sf::Event::Resized) {
                // update the view to the new size of the window
                sf::FloatRect visibleArea(0.f,
                                          0.f,
                                          static_cast<float>(event.size.width),
                                          static_cast<float>(event.size.height));
                m_window.setView(sf::View(visibleArea));
            }
        }

        auto deltaTimeToMs = static_cast<int>(deltaTimeS * 1000.f);
        for (int tickIndex = 0; tickIndex < engineTicks; tickIndex++) {
            m_engine.poll(std::chrono::milliseconds(deltaTimeToMs));
        }
        return result;
    }

    /**
     * @brief Get the grid size in world space after scaling is applied
     **/
    sf::Vector2f gridCellSize() const {
        sf::Vector2f result(20.f * m_gridScale, 20.f * m_gridScale);
        return result;
    }

    sf::Vector2f       m_gridStartOffset;
    aos::jack::Engine &m_engine;
    int                m_gridSizeX;
    int                m_gridSizeY;
    float              m_gridScale;
    sf::RenderWindow   m_window;
    unsigned int       m_frameRate;
};
}; // namespace aos

