#include "sensory_selection_validation.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "alien_evolution/evolution/Reproduction.hpp"

namespace
{
    struct Statistics
    {
        std::size_t size = 0, failures = 0;
        double fitness = b6::missing(), maximumFitness = b6::missing();
        double meanK = b6::missing(), medianK = b6::missing(), meanLogK = b6::missing();
        double minimumK = b6::missing(), q10 = b6::missing(), q90 = b6::missing(), maximumK = b6::missing();
        double material = b6::missing(), boundary = b6::missing(), netEnergy = b6::missing();
        double modulation = b6::missing();
    };

    std::ofstream output(const std::filesystem::path& path)
    {
        std::ofstream stream(path);
        if (!stream) throw std::runtime_error("Cannot open output: " + path.string());
        stream.exceptions(std::ios::badbit | std::ios::failbit);
        stream << std::setprecision(17);
        return stream;
    }

    std::string csvQuote(const std::string& value)
    {
        std::string result = "\"";
        for (char character : value)
        {
            result += character;
            if (character == '"') result += '"';
        }
        return result + '"';
    }

    double modulation(const ae::HeritableProgram& program, double signal)
    {
        return program.sensoryProgram().makeRegulatoryInputInterface(program.regulatoryProgram())
            .modulationFactor(3, {{b6::signalIds.resourceSignalId, signal}});
    }

    Statistics evaluatePopulation(ae::Population& population, double resource,
        std::size_t replicate, std::uint64_t seed, const char* treatment,
        std::size_t generation, std::ostream& failures)
    {
        Statistics result;
        result.size = population.size();
        if (population.empty()) return result;
        std::vector<double> fitness, ks, logs, materials, boundaries, energies, responses;
        for (std::size_t i = 0; i < population.size(); ++i)
        {
            auto& organism = population.at(i);
            const auto& program = organism.heritableProgram();
            b6::checkFixedComponents(program); // Isolation violations abort; never convert to fitness.
            const double k = program.sensoryProgram().channels()[0].halfSaturation;
            ks.push_back(k);
            logs.push_back(std::log(k));
            responses.push_back(modulation(program, resource));
            try
            {
                auto evaluation = b6::evaluate(program, resource);
                organism.setPhenotype(std::move(evaluation.phenotype));
                organism.setFitness(evaluation.fitness);
                fitness.push_back(evaluation.fitness);
                materials.push_back(evaluation.metrics.totalMaterial);
                boundaries.push_back(evaluation.metrics.exposedBoundary);
                energies.push_back(evaluation.energy.netEnergy);
            }
            catch (const std::exception& error)
            {
                ++result.failures;
                organism.clearEvaluation();
                organism.setFitness(0.0);
                fitness.push_back(0.0);
                // Failed phenotype metrics are unavailable, not invented zeros.
                materials.push_back(b6::missing());
                boundaries.push_back(b6::missing());
                energies.push_back(b6::missing());
                failures << resource << ',' << replicate << ',' << seed << ',' << treatment << ','
                    << generation << ',' << i << ',' << csvQuote(error.what()) << '\n';
            }
        }
        result.fitness = b6::mean(fitness);
        result.maximumFitness = *std::max_element(fitness.begin(), fitness.end());
        result.meanK = b6::mean(ks);
        result.medianK = b6::quantile(ks, 0.5);
        result.meanLogK = b6::mean(logs);
        result.minimumK = *std::min_element(ks.begin(), ks.end());
        result.maximumK = *std::max_element(ks.begin(), ks.end());
        result.q10 = b6::quantile(ks, 0.1);
        result.q90 = b6::quantile(ks, 0.9);
        result.material = b6::mean(materials);
        result.boundary = b6::mean(boundaries);
        result.netEnergy = b6::mean(energies);
        result.modulation = b6::mean(responses);
        return result;
    }

    void writeStatistics(std::ostream& stream, const Statistics& result)
    {
        stream << result.size << ',' << result.fitness << ',' << result.maximumFitness << ','
            << (result.size == 0) << ',' << result.failures << ',' << result.meanK << ',' << result.medianK << ','
            << result.meanLogK << ',' << result.minimumK << ',' << result.q10 << ',' << result.q90 << ','
            << result.maximumK << ',' << result.material << ',' << result.boundary << ','
            << result.netEnergy << ',' << result.modulation;
    }

    struct Outcome
    {
        Statistics final;
        std::size_t failures;
        double adaptiveShift;
    };

    void assay(const ae::Population& population, double trainingResource,
        const std::vector<double>& environments, std::size_t replicate, std::uint64_t seed,
        const char* treatment, std::ostream& responseCsv, std::ostream& finalCsv)
    {
        for (double probe : b6::responseProbes)
        {
            std::vector<double> responses;
            for (const auto& organism : population.organisms())
                responses.push_back(modulation(organism.heritableProgram(), probe));
            responseCsv << "modulation," << trainingResource << ',' << probe << ',' << replicate << ',' << seed
                << ',' << treatment << ',' << population.size() << ',' << b6::mean(responses) << ','
                << b6::sd(responses) << ",,,,\n";
        }
        for (double assayEnvironment : environments)
        {
            std::vector<double> fitness, material, boundary, energy;
            for (std::size_t i = 0; i < population.size(); ++i)
            {
                const auto& program = population.at(i).heritableProgram();
                const auto value = b6::evaluate(program, assayEnvironment);
                fitness.push_back(value.fitness);
                material.push_back(value.metrics.totalMaterial);
                boundary.push_back(value.metrics.exposedBoundary);
                energy.push_back(value.energy.netEnergy);
                finalCsv << trainingResource << ',' << assayEnvironment << ',' << replicate << ',' << seed << ','
                    << treatment << ',' << i << ',' << program.sensoryProgram().channels()[0].halfSaturation << ','
                    << value.fitness << ',' << value.metrics.totalMaterial << ',' << value.metrics.exposedBoundary << ','
                    << value.energy.netEnergy << ',' << modulation(program, assayEnvironment) << '\n';
            }
            responseCsv << "phenotype," << trainingResource << ',' << assayEnvironment << ',' << replicate << ','
                << seed << ',' << treatment << ',' << population.size() << ",,," << b6::mean(fitness) << ','
                << b6::mean(material) << ',' << b6::mean(boundary) << ',' << b6::mean(energy) << '\n';
        }
    }

    Outcome run(double resource, int direction, const std::vector<double>& assayEnvironments,
        std::size_t replicate, std::uint64_t seed, const char* treatment,
        ae::SelectionMode selection, bool mutate, std::ostream& raw, std::ostream& runs,
        std::ostream& responses, std::ostream& finals, std::ostream& failures)
    {
        ae::Random random(seed);
        std::vector<ae::Organism> founders;
        for (std::size_t i = 0; i < b6::populationSize; ++i) founders.emplace_back(b6::founder());
        ae::Population population(std::move(founders));
        Statistics final;
        std::size_t totalFailures = 0;
        const auto mutation = b6::mutationConfig(mutate);
        for (std::size_t generation = 0; generation <= b6::generations; ++generation)
        {
            final = evaluatePopulation(population, resource, replicate, seed, treatment, generation, failures);
            totalFailures += final.failures;
            raw << resource << ',' << direction << ',' << replicate << ',' << seed << ',' << treatment << ',' << generation << ',';
            writeStatistics(raw, final);
            raw << ',' << totalFailures << '\n';
            if (generation < b6::generations)
                population = ae::reproducePopulation(population, b6::populationSize, mutation, random, selection);
        }
        const double shift = b6::adaptiveShift(direction, final.meanLogK);
        runs << resource << ',' << direction << ',' << replicate << ',' << seed << ',' << treatment << ',' << b6::generations << ',';
        writeStatistics(runs, final);
        runs << ',' << totalFailures << ',' << shift << '\n';
        // Assays are read-only, use no RNG, and happen after the evolution phase.
        assay(population, resource, assayEnvironments, replicate, seed, treatment, responses, finals);
        return {final, totalFailures, shift};
    }

    void summarize(double resource, const char* comparison, const std::vector<double>& observations,
        std::ostream& csv)
    {
        std::vector<double> valid;
        std::size_t positive = 0, negative = 0, ties = 0;
        for (double value : observations)
        {
            if (!std::isfinite(value)) continue;
            valid.push_back(value);
            if (value > b6::tieTolerance) ++positive;
            else if (value < -b6::tieTolerance) ++negative;
            else ++ties;
        }
        const double average = b6::mean(valid), deviation = b6::sd(valid);
        const double halfWidth = valid.empty() ? b6::missing() : 1.96 * deviation / std::sqrt(static_cast<double>(valid.size()));
        csv << resource << ',' << comparison << ',' << valid.size() << ',' << observations.size() - valid.size() << ','
            << average << ',' << deviation << ',' << average - halfWidth << ',' << average + halfWidth << ','
            << positive << ',' << negative << ',' << ties << '\n';
        std::cout << resource << ' ' << comparison << ": mean=" << average << " SD=" << deviation
            << " approx95%=[" << average - halfWidth << ',' << average + halfWidth << "] +/−/tie="
            << positive << '/' << negative << '/' << ties << " unavailable=" << observations.size() - valid.size() << '\n';
    }
}

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2 || argc > 3 || (std::string(argv[1]) != "--landscape-only" && std::string(argv[1]) != "--full"))
        {
            std::cerr << "Usage: AlienEvolutionSensorySelectionValidation --landscape-only|--full [output-directory]\n";
            return 2;
        }
        const bool full = std::string(argv[1]) == "--full";
        const std::filesystem::path directory = argc == 3 ? argv[2] : "experiments/output";
        std::filesystem::create_directories(directory);
        if (full && std::filesystem::exists(directory / "sensory_selection_runs.csv"))
            throw std::runtime_error("Refusing to overwrite a full evolutionary run; use a separate output directory for replay.");
        const auto started = std::chrono::steady_clock::now();
        std::vector<b6::Landscape> candidates;
        for (double resource : b6::candidateResources) candidates.push_back(b6::scan(resource));
        const auto selected = b6::selectEnvironments(candidates);
        std::ostringstream landscape;
        landscape << std::setprecision(17)
            << "resource,half_saturation,fitness,total_material,exposed_boundary,net_energy,founder_fitness,lower_probe_fitness,higher_probe_fitness,gradient,gradient_sign,grid_optimum_k,grid_optimum_fitness,optimum_on_boundary,tested_environment,primary_environment\n";
        for (std::size_t c = 0; c < candidates.size(); ++c)
        {
            const auto& candidate = candidates[c];
            std::cout << std::setprecision(17) << "Landscape resource=" << candidate.resource << " founder_fitness="
                << candidate.founderFitness << " gradient=" << candidate.gradient << " direction=" << candidate.direction
                << " grid_optimum_K=" << candidate.optimumK << " boundary=" << candidate.boundary << '\n';
            for (std::size_t i = 0; i < candidate.ks.size(); ++i)
            {
                const auto& value = candidate.evaluations[i];
                landscape << candidate.resource << ',' << candidate.ks[i] << ',' << value.fitness << ','
                    << value.metrics.totalMaterial << ',' << value.metrics.exposedBoundary << ',' << value.energy.netEnergy << ','
                    << candidate.founderFitness << ',' << candidate.lowerFitness << ',' << candidate.higherFitness << ','
                    << candidate.gradient << ',' << candidate.direction << ',' << candidate.optimumK << ','
                    << candidate.optimumFitness << ',' << candidate.boundary << ','
                    << (std::find(selected.begin(), selected.end(), c) != selected.end()) << ','
                    << (!selected.empty() && selected[0] == c) << '\n';
            }
        }
        const auto landscapePath = directory / "sensory_selection_landscape.csv";
        if (full)
        {
            std::ifstream frozen(landscapePath);
            std::ostringstream previous;
            previous << frozen.rdbuf();
            if (!frozen || previous.str() != landscape.str())
                throw std::runtime_error("Full run requires a matching previously frozen --landscape-only CSV.");
        }
        else
        {
            auto stream = output(landscapePath);
            stream << landscape.str();
        }
        if (selected.empty())
        {
            std::cout << "No identifiable viable sensory-selection benchmark: all viable gradients are effectively zero. No evolution run.\n";
            return 0;
        }
        std::cout << "Frozen primary resource=" << candidates[selected[0]].resource << " direction="
            << candidates[selected[0]].direction << " opposite-gradient contrast=" << (selected.size() == 2) << '\n';
        if (!full) return 0;

        auto raw = output(directory / "sensory_selection_raw.csv");
        auto runs = output(directory / "sensory_selection_runs.csv");
        auto responses = output(directory / "sensory_selection_response_assay.csv");
        auto finals = output(directory / "sensory_selection_final.csv");
        auto failures = output(directory / "sensory_selection_failures.csv");
        auto summary = output(directory / "sensory_selection_summary.csv");
        const std::string columns = "resource,expected_direction,replicate,seed,treatment,generation,population_size,mean_fitness,maximum_fitness,extinct,development_failures,mean_k,median_k,mean_log_k,min_k,q10_k,q90_k,max_k,mean_material,mean_boundary,mean_net_energy,mean_training_modulation,cumulative_development_failures";
        raw << columns << '\n';
        runs << columns << ",adaptive_shift\n";
        responses << "assay,training_resource,assay_signal_or_resource,replicate,seed,treatment,population_size,mean_modulation,sd_modulation,mean_fitness,mean_material,mean_boundary,mean_net_energy\n";
        finals << "training_resource,assay_resource,replicate,seed,treatment,organism,half_saturation,fitness,total_material,exposed_boundary,net_energy,modulation\n";
        failures << "resource,replicate,seed,treatment,generation,organism,error\n";
        summary << "resource,comparison,n_available,n_unavailable,mean,sample_sd,approx95_lower,approx95_upper,positive,negative,tie\n";
        std::vector<double> assayEnvironments;
        for (auto index : selected) assayEnvironments.push_back(candidates[index].resource);
        for (auto index : selected)
        {
            const auto& candidate = candidates[index];
            std::vector<double> primary, selectedShift, neutralShift, fitnessDifference, noMutationDifference,
                modulationDifference, materialDifference, boundaryDifference;
            std::size_t extinct[3]{}, failureCounts[3]{};
            for (std::size_t replicate = 0; replicate < b6::replicates; ++replicate)
            {
                const auto seed = b6::baseSeed + b6::seedStride * replicate;
                const auto selectedRun = run(candidate.resource, candidate.direction, assayEnvironments, replicate, seed,
                    "selected_mutation", ae::SelectionMode::FitnessProportional, true, raw, runs, responses, finals, failures);
                const auto neutralRun = run(candidate.resource, candidate.direction, assayEnvironments, replicate, seed,
                    "neutral_mutation", ae::SelectionMode::Uniform, true, raw, runs, responses, finals, failures);
                const auto noMutation = run(candidate.resource, candidate.direction, assayEnvironments, replicate, seed,
                    "selected_no_mutation", ae::SelectionMode::FitnessProportional, false, raw, runs, responses, finals, failures);
                const Outcome outcomes[]{selectedRun, neutralRun, noMutation};
                for (int treatment = 0; treatment < 3; ++treatment)
                {
                    extinct[treatment] += outcomes[treatment].final.size == 0;
                    failureCounts[treatment] += outcomes[treatment].failures;
                }
                primary.push_back(selectedRun.adaptiveShift - neutralRun.adaptiveShift);
                selectedShift.push_back(selectedRun.adaptiveShift);
                neutralShift.push_back(neutralRun.adaptiveShift);
                fitnessDifference.push_back(selectedRun.final.fitness - neutralRun.final.fitness);
                noMutationDifference.push_back(selectedRun.final.fitness - noMutation.final.fitness);
                modulationDifference.push_back(selectedRun.final.modulation - neutralRun.final.modulation);
                materialDifference.push_back(selectedRun.final.material - neutralRun.final.material);
                boundaryDifference.push_back(selectedRun.final.boundary - neutralRun.final.boundary);
                std::cout << "Completed resource=" << candidate.resource << " replicate=" << replicate + 1 << '/' << b6::replicates << std::endl;
            }
            summarize(candidate.resource, "adaptive_shift_selected_minus_neutral", primary, summary);
            summarize(candidate.resource, "adaptive_shift_selected", selectedShift, summary);
            summarize(candidate.resource, "adaptive_shift_neutral", neutralShift, summary);
            summarize(candidate.resource, "fitness_selected_minus_neutral", fitnessDifference, summary);
            summarize(candidate.resource, "fitness_selected_minus_no_mutation", noMutationDifference, summary);
            summarize(candidate.resource, "training_modulation_selected_minus_neutral", modulationDifference, summary);
            summarize(candidate.resource, "material_selected_minus_neutral", materialDifference, summary);
            summarize(candidate.resource, "boundary_selected_minus_neutral", boundaryDifference, summary);
            std::cout << "Extinct selected/neutral/no_mutation=" << extinct[0] << '/' << extinct[1] << '/' << extinct[2]
                << " development_failures=" << failureCounts[0] << '/' << failureCounts[1] << '/' << failureCounts[2] << '\n';
        }
        std::cout << "Completed full B6 in " << std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count() << " seconds.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "B6 VALIDATION FAILURE: " << error.what() << '\n';
        return 1;
    }
}
