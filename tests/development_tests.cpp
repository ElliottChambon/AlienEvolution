#include <cmath>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <vector>

#include "alien_evolution/development/Development.hpp"
#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/Genome.hpp"

namespace
{

    void require(
        const bool condition,
        const char* message
    )
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    bool phenotypesEqual(
        const ae::Phenotype& a,
        const ae::Phenotype& b,
        const double tolerance = 1.0e-12
    )
    {
        if (
            a.width() != b.width()
            || a.height() != b.height()
            )
        {
            return false;
        }

        for (std::size_t y = 0; y < a.height(); ++y)
        {
            for (std::size_t x = 0; x < a.width(); ++x)
            {
                if (
                    std::abs(
                        a.materialAt(x, y)
                        - b.materialAt(x, y)
                    ) > tolerance
                    )
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool occupiedMaterialIsConnected(
        const ae::Phenotype& phenotype
    )
    {
        const std::size_t width = phenotype.width();
        const std::size_t height = phenotype.height();

        std::vector<bool> visited(width * height, false);
        std::queue<std::pair<std::size_t, std::size_t>> frontier;

        bool foundStart = false;
        std::size_t occupiedCount = 0;

        for (std::size_t y = 0; y < height; ++y)
        {
            for (std::size_t x = 0; x < width; ++x)
            {
                if (phenotype.materialAt(x, y) > 0.0)
                {
                    ++occupiedCount;

                    if (!foundStart)
                    {
                        frontier.emplace(x, y);
                        visited[y * width + x] = true;
                        foundStart = true;
                    }
                }
            }
        }

        if (!foundStart)
        {
            return false;
        }

        std::size_t reachedCount = 0;

        constexpr int directions[8][2] =
        {
            {-1, -1}, {0, -1}, {1, -1},
            {-1,  0},          {1,  0},
            {-1,  1}, {0,  1}, {1,  1}
        };

        while (!frontier.empty())
        {
            const auto [x, y] = frontier.front();
            frontier.pop();

            ++reachedCount;

            for (const auto& direction : directions)
            {
                const int nx =
                    static_cast<int>(x) + direction[0];

                const int ny =
                    static_cast<int>(y) + direction[1];

                if (
                    nx < 0
                    || ny < 0
                    || nx >= static_cast<int>(width)
                    || ny >= static_cast<int>(height)
                    )
                {
                    continue;
                }

                const std::size_t ux =
                    static_cast<std::size_t>(nx);

                const std::size_t uy =
                    static_cast<std::size_t>(ny);

                const std::size_t index =
                    uy * width + ux;

                if (
                    !visited[index]
                    && phenotype.materialAt(ux, uy) > 0.0
                    )
                {
                    visited[index] = true;
                    frontier.emplace(ux, uy);
                }
            }
        }

        return reachedCount == occupiedCount;
    }

} // namespace

int main()
{
    try
    {
        const ae::Environment environment{};
        const ae::Genome genome{};

        const ae::Development development(
            40,
            30,
            10
        );

        const ae::Phenotype first =
            development.develop(genome, environment);

        const ae::Phenotype second =
            development.develop(genome, environment);

        // Test 1: development produces organismal material.
        require(
            first.totalMaterial() > 0.0,
            "Development produced no material."
        );

        // Test 2: reference development must remain bounded.
        require(
            first.totalMaterial()
            < static_cast<double>(
                first.width() * first.height()
                ),
            "Reference development filled the entire grid."
        );

        // Test 3: material occupancy always remains valid.
        for (std::size_t y = 0; y < first.height(); ++y)
        {
            for (std::size_t x = 0; x < first.width(); ++x)
            {
                const double material =
                    first.materialAt(x, y);

                require(
                    material >= 0.0 && material <= 1.0,
                    "Phenotype material escaped [0, 1]."
                );
            }
        }

        // Test 4: deterministic development.
        require(
            phenotypesEqual(first, second),
            "Identical inputs produced different phenotypes."
        );

        // Test 5: organismal material remains connected.
        require(
            occupiedMaterialIsConnected(first),
            "Developed phenotype contains disconnected material."
        );

        std::cout
            << "All development tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "TEST FAILURE: "
            << error.what()
            << '\n';

        return 1;
    }
}