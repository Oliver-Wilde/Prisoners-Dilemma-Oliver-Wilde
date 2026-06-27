#include "MetaWriter.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {
    void jsonString(std::ostream& os, const std::string& s) {
        os << '"';
        for (unsigned char c : s) {
            switch (c) {
            case '"':  os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            case '\b': os << "\\b";  break;
            case '\f': os << "\\f";  break;
            case '\n': os << "\\n";  break;
            case '\r': os << "\\r";  break;
            case '\t': os << "\\t";  break;
            default:
                if (c < 0x20) {
                    std::ios::fmtflags save = os.flags();
                    char fill = os.fill();
                    os << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0') << int(c)
                        << std::dec << std::setfill(fill);
                    os.flags(save);
                }
                else {
                    os << c;
                }
            }
        }
        os << '"';
    }

    void jsonStringArray(std::ostream& os, const std::vector<std::string>& v) {
        os << "[";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) os << ", ";
            jsonString(os, v[i]);
        }
        os << "]";
    }
}

void MetaWriter::writeScoresMeta(const std::string& filename,
    const Config& cfg,
    const std::string& label,
    double epsilonUsed,
    const std::vector<std::string>& strategies,
    const Payoffs& p)
{
    std::ofstream f(filename, std::ios::binary);
    if (!f) return;

    f << "{\n";
    f << "  \"label\": "; jsonString(f, label); f << ",\n";
    f << "  \"rounds\": " << cfg.rounds << ",\n";
    f << "  \"repeats\": " << cfg.repeats << ",\n";
    f << "  \"seed\": " << cfg.seed << ",\n";
    f << "  \"epsilon\": " << std::setprecision(10) << epsilonUsed << ",\n";
    f << "  \"payoffs\": {"
        << "\"T\": " << p.T << ", \"R\": " << p.R
        << ", \"P\": " << p.P << ", \"S\": " << p.S << "},\n";
    f << "  \"format\": "; jsonString(f, cfg.format); f << ",\n";
    f << "  \"strategies\": "; jsonStringArray(f, strategies); f << ",\n";
    f << "  \"flags\": {"
        << "\"evolve\": " << (cfg.evolve ? "true" : "false") << ", "
        << "\"scb\": " << (cfg.scb ? "true" : "false") << "}\n";
    f << "}\n";
}

void MetaWriter::writeEvolutionMeta(const std::string& filename,
    const Config& cfg,
    const std::vector<std::string>& strategies,
    const Payoffs& p)
{
    std::ofstream f(filename, std::ios::binary);
    if (!f) return;

    f << "{\n";
    f << "  \"label\": \"Q4\",\n";
    f << "  \"generations\": " << cfg.generations << ",\n";
    f << "  \"mutation\": " << std::setprecision(10) << cfg.mutation << ",\n";
    f << "  \"rounds\": " << cfg.rounds << ",\n";
    f << "  \"seed\": " << cfg.seed << ",\n";
    f << "  \"epsilon\": " << std::setprecision(10) << cfg.epsilon << ",\n";
    f << "  \"payoffs\": {"
        << "\"T\": " << p.T << ", \"R\": " << p.R
        << ", \"P\": " << p.P << ", \"S\": " << p.S << "},\n";
    f << "  \"strategies\": "; jsonStringArray(f, strategies); f << "\n";
    f << "}\n";
}
