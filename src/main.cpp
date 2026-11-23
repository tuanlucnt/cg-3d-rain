#include "Application.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

int main(int argc, char** argv) {
    try {
        RunOptions options;
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--smoke-test") options.smokeTest = true;
            else if (argument == "--no-fog") options.noFog = true;
            else if (argument == "--auto-time") options.autoTime = true;
            else if ((argument == "--hour" || argument == "--day-duration") && i+1<argc) {
                const std::string value=argv[++i];
                std::size_t n=0;
                const double parsed=std::stod(value,&n);
                if (n!=value.size() || !std::isfinite(parsed)) throw std::invalid_argument("Time argument must be a finite number");
                if (argument=="--hour") {
                    if (parsed<0 || parsed>24) throw std::invalid_argument("Hour must be in [0,24]");
                    options.hour=parsed;
                } else {
                    if (parsed<60 || parsed>3600) throw std::invalid_argument("Day duration must be in [60,3600] seconds");
                    options.dayDuration=parsed;
                }
            }
            else if (argument == "--fog-density" && i + 1 < argc) {
                const std::string value = argv[++i];
                std::size_t n = 0;
                options.fogDensity = std::stof(value, &n);
                if (n != value.size() || !std::isfinite(options.fogDensity) ||
                    options.fogDensity < 0 || options.fogDensity > 0.1f)
                    throw std::invalid_argument("Fog density must be in [0, 0.1]");
            }
            else if (argument == "--capture" && i + 1 < argc) options.capturePath = argv[++i];
            else if (argument == "--capture-lightning" && i + 1 < argc)
                options.lightningCapturePath = argv[++i];
            else if (argument == "--capture-cloud-wrap" && i + 1 < argc)
                options.cloudWrapCapturePath = argv[++i];
            else if (argument == "--capture-views" && i + 1 < argc)
                options.viewCaptureDirectory = argv[++i];
            else if ((argument == "--benchmark" || argument == "--benchmark-unlocked") && i + 1 < argc) {
                options.unlocked = argument == "--benchmark-unlocked";
                const std::string value = argv[++i];
                std::size_t consumed = 0;
                options.benchmarkSeconds = std::stod(value, &consumed);
                if (consumed != value.size() || !std::isfinite(options.benchmarkSeconds) ||
                    options.benchmarkSeconds <= 0 || options.benchmarkSeconds > 600)
                    throw std::invalid_argument("--benchmark requires seconds in (0, 600]");
            } else if (argument == "--help") {
                std::cout << "Usage: rain_thunderstorm [--smoke-test [--capture frame.ppm] "
                             "[--capture-lightning flash.ppm] [--capture-cloud-wrap wrap.ppm]]\n"
                          << "       rain_thunderstorm --benchmark <seconds, max 600>\n"
                          << "       rain_thunderstorm --benchmark-unlocked <seconds> [--no-fog]\n"
                          << "       --fog-density <0..0.1> | --capture-views <directory> (smoke only)\n"
                          << "       --hour <0..24> | --auto-time | --day-duration <60..3600 seconds>\n"
                          << "WASD/QE: move; left mouse drag: look; 1/2/3: rain/lightning/clouds; "
                             "L: flash; F: fog; 4/5/6: camera presets; Space: pause; R: reset; Esc: exit\n"
                          << "N: day/night; [ / ]: hour; T: auto time; H: time/help\n";
                return 0;
            } else {
                throw std::invalid_argument("Unknown or incomplete argument: " + argument);
            }
        }
        if ((!options.capturePath.empty() || !options.lightningCapturePath.empty() ||
             !options.cloudWrapCapturePath.empty() || !options.viewCaptureDirectory.empty()) && !options.smokeTest)
            throw std::invalid_argument("Capture options require --smoke-test");
        if (options.smokeTest && options.benchmarkSeconds > 0)
            throw std::invalid_argument("Choose either --smoke-test or --benchmark");
        Application application(std::move(options));
        return application.run(argv[0]);
    } catch (const std::exception& e) {
        std::cerr << "Application failed: " << e.what() << '\n';
        return 1;
    }
}
