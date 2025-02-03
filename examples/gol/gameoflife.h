// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#pragma once

/// @brief  Simulate the Game of Life
struct GameOfLife
{
    GameOfLife(int gridSize)
    : m_grid(gridSize * gridSize)
    , m_population(gridSize * gridSize)
    , m_gridSize(gridSize)
    {
        // create a glider
        // clear
        for (int i = 0; i < gridSize*gridSize; ++i) {
            m_grid[i] = false;
        }

        size_t ts = gridSize/10;

        addGlider(ts * 5, ts * 2);
        addGlider(ts * 3, ts * 7);
        addGlider(ts * 2, ts * 2);
        addGlider(ts * 8, ts * 4);

        update();
    }

    // 010
    // 001
    // 111
    void addGlider(int x, int y)
    {
        size_t i = (y * m_gridSize) + x;
        m_grid[(i - m_gridSize)] = true;
        m_grid[i + 1] = true;
        m_grid[(i + m_gridSize) - 1] = true;
        m_grid[(i + m_gridSize) + 1] = true;
        m_grid[(i + m_gridSize)] = true;
    }

    void update()
    {
        // calculate the population for all cells
        int gridSize = m_gridSize;
        for(int r = 0; r < gridSize; ++r) {
            for(int c = 0; c < gridSize; ++c) {
                int i = c + (r * gridSize);
                int i_n = i - gridSize;
                int i_s = i + gridSize;
                int east = 1;
                int west = -1;
                int population = 0;

                if (r==0) i_n = ((gridSize - 1) * gridSize) + c;
                if (r==(gridSize-1)) i_s = c;
                if (c==0) west = gridSize - 1;
                if (c==(gridSize-1)) east = -(gridSize - 1);

                // up
                if (m_grid[i_n+west])
                    population++;

                if (m_grid[i_n+east])
                    population++;

                if (m_grid[i_n])
                    population++;

                // mid
                if (m_grid[i+east])
                    population++;

                if (m_grid[i+west])
                    population++;

                // bottom
                if (m_grid[i_s+west])
                    population++;

                if (m_grid[i_s+east])
                    population++;

                if (m_grid[i_s])
                    population++;

                m_population[i] = population;
            }
        }
    }

    std::vector<bool>  m_grid;
    std::vector<int>   m_population;
    int                m_gridSize;
};
