#include "alien_evolution/physics/ReversibleChemicalAssociationFiniteBath.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace
{
    double uniformVolumeRadius(double a, double R, ae::Random& random)
    {
        const auto a3 = a * a * a;
        const auto R3 = R * R * R;
        return std::cbrt(a3 + random.uniform01() * (R3 - a3));
    }

    ae::ChemicalAssociationParameters parameters(double chi, double delta)
    {
        constexpr double a = 1.0;
        constexpr double D = 1.0;
        return {a, D, 4.0 * std::numbers::pi * chi, delta};
    }
}

int main()
{
    try
    {
        constexpr double lambda = 2.0;
        constexpr int replicates = 10;
        constexpr double horizon = 30.0;
        const std::vector<std::size_t> ligandCounts{1, 2, 5};
        const std::vector<double> chis{1.0, 10.0};
        const std::vector<double> deltas{1.0, 10.0};
        const std::vector<double> tauMins{1.0e-4, 1.0e-5, 1.0e-6};

        std::cout << "CHEM-1C resolution scan\n";
        std::cout << "tau_min,N,chi,delta,reactive_modes,reflecting_modes,expected_bound,observed_bound,resolved,unresolved\n";

        std::uint64_t seed = 490100;
        for (const auto tauMin : tauMins)
        for (const auto N : ligandCounts)
        {
            for (const auto chi : chis)
            {
                for (const auto delta : deltas)
                {
                    ae::FiniteChemicalBathConfig config;
                    config.outerRadius = lambda;
                    config.minimumResolvedDimensionlessTime = tauMin;
                    config.spectralTolerance = 1.0e-12;
                    config.maxModes = 32768;

                    const ae::ReversibleChemicalAssociationFiniteBath bath(
                        parameters(chi, delta),
                        config);

                    int bound = 0;
                    int resolved = 0;
                    int unresolved = 0;
                    for (int replicate = 0; replicate < replicates; ++replicate)
                    {
                        ae::Random random(seed++);
                        ae::FiniteChemicalBathState state;
                        state.ligandRadii.reserve(N);
                        for (std::size_t i = 0; i < N; ++i)
                            state.ligandRadii.push_back(uniformVolumeRadius(1.0, lambda, random));

                        try
                        {
                            const auto trajectory =
                                bath.sampleTrajectory(state, horizon, random);
                            ++resolved;
                            bound += trajectory.finalState.boundLigand ? 1 : 0;
                        }
                        catch (const std::domain_error&)
                        {
                            // Numerical-resolution failures are a benchmark result,
                            // not something to silently repair with another solver.
                            ++unresolved;
                        }
                    }

                    const auto observed =
                        resolved == 0
                        ? std::numeric_limits<double>::quiet_NaN()
                        : static_cast<double>(bound) / static_cast<double>(resolved);

                    std::cout
                        << tauMin << ','
                        << N << ','
                        << chi << ','
                        << delta << ','
                        << bath.diagnostics().reactiveModeCount << ','
                        << bath.diagnostics().reflectingModeCount << ','
                        << std::setprecision(10)
                        << bath.equilibriumBoundProbability(N) << ','
                        << observed << ','
                        << resolved << ','
                        << unresolved << '\n';
                }
            }
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "VALIDATION FAILURE: " << error.what() << '\n';
        return 1;
    }
}
