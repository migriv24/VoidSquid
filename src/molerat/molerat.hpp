/* molerat.hpp — Void MoleRat's pure half.
 *
 * okf/concepts/molerat.md; molerat/design.md.
 *
 * MoleRat is a forge, not an artifact. This library takes a normalized "foreign IR"
 * (what a disassembler or a reader produced), recovers the structure it can, states a
 * mathematical model and a mapping onto Void Core's vocabulary, names the residual, and
 * emits a state document Void Squid can open.
 *
 * ── Pure, like squid_read and squid_sim ──
 *
 * This target links the standard library and the vendored JSON header, and nothing else:
 * no Void Core, no Void Jelly, no Void Maiz, no filesystem, no network, no clock, no RNG.
 * The host (src/molerat/effect.cpp) reads the IR and writes the document; given the same
 * inputs, run id and seed, this library produces byte-identical output.
 *
 * ── Honesty ──
 *
 * Every model, mapping, residual and holiday carries evidence and a confidence. The
 * fixture is synthetic, and the residual says so. The emitted document is valid Void Core,
 * but a research claim is a proposal until a person or a rule accepts it.
 */
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace squid::molerat {

struct Target {
    std::string id;
    std::string title;
    std::string platform;
    std::string origin;
    std::string depth = "source";   // bytes | opaque | source | structure
};

struct Op {
    std::string kind;    // load | store | add | sub | cmp | branch | call | return
    std::string dst;
    std::string src;
    std::string value;
};

struct Block {
    std::string id;
    std::vector<Op> ops;
};

struct Function {
    std::string name;
    std::string addr;
    std::vector<Block> blocks;
    std::vector<std::string> calls;
    std::vector<std::string> reads;
    std::vector<std::string> writes;
    std::vector<std::string> evidence;
};

struct Field {
    std::string name;
    std::string type;
    std::string unit;
    double value = 0.0;
    bool has_value = false;
};

struct Table {
    std::string name;
    std::vector<Field> fields;
};

struct ForeignIR {
    std::string schema;
    Target target;
    std::vector<Function> functions;
    std::vector<Table> tables;
    std::vector<std::string> constants;
    std::vector<std::string> residual;
};

/* What the IR says about itself: reads, writes, calls, a call order, and one
 * evidence sentence per function. Still pure; no interpretation of the target's
 * domain results. */
struct Analysis {
    std::vector<std::string> order;
    std::map<std::string, std::vector<std::string>> reads;
    std::map<std::string, std::vector<std::string>> writes;
    std::map<std::string, std::vector<std::string>> calls;
    std::vector<std::string> findings;
};

struct ModelRecord {
    std::string name;
    std::string source;
    std::string domain;
    std::vector<std::string> operations;
    std::vector<std::string> laws;
    std::vector<std::string> composition;
    std::string observation;
    std::vector<std::string> citations;
    std::string confidence;
    std::string evidence;
};

struct MappingRecord {
    std::string name;
    std::string source_construct;
    std::string void_shape;
    std::vector<std::string> fields;
    std::vector<std::string> lost;
    std::string confidence;
    std::string evidence;
};

struct ResidualRecord {
    std::string name;
    std::string what;
    std::string why;
    std::string evidence;
    std::string closest;
    std::string confidence;
};

struct HolidayRecord {
    std::string name;
    std::string target;
    std::string iface;
    std::string channels;
    std::string lifecycle;
    std::string ownership;
    std::string units;
    std::string not_covered;
};

struct Research {
    std::vector<ModelRecord> models;
    std::vector<MappingRecord> mappings;
    std::vector<ResidualRecord> residuals;
    std::vector<HolidayRecord> holidays;
};

struct RuneSpec {
    std::string name;
    std::string glyph;
    std::string id;
    std::string who;
    std::string what;
    std::string when;
    std::string where;
    std::string why;
    std::string how;
    std::vector<std::string> tags;
    std::string content_json;    // a JSON object, serialized
    std::string quantity_json;   // empty = the field is absent
};

struct LinkSpec {
    std::string from;
    std::string to;
    std::string relation;
    double weight = 1.0;
    bool directed = true;
};

struct MantleSpec {
    std::string name;
    std::string domain;
    std::vector<RuneSpec> runes;
    std::vector<LinkSpec> links;
};

struct ComposeResult {
    MantleSpec game;
    MantleSpec research;
    std::vector<std::string> scaffolding;   // names the normal form eliminated
    bool ok = false;
    std::string error;
};

struct Validation {
    bool ok = false;
    std::vector<std::string> problems;
};

struct Slice {
    ForeignIR ir;
    Analysis analysis;
    Research research;
    ComposeResult compose;
    std::string document;
    Validation validation;
    std::vector<std::string> residual;
};

/* Parse and bound-check a molerat.foreign-ir/0.1 document. */
bool parse_ir(std::string_view text, ForeignIR& out, std::string& error);

/* Merge the explicit read/write/call lists with what the ops themselves say, and
 * order the functions by their calls. */
Analysis analyze(const ForeignIR& ir);

/* Produce the four research record kinds deterministically. The agent harness may
 * refine them later; this is the honest first pass. */
Research research(const ForeignIR& ir, const Analysis& analysis);

/* Map the research onto two mantles: <run>:game and molerat:<run>. The normal form
 * contains no scaffolding; the scaffolding names are returned so a host can show what
 * the Reduce ruleset removed. */
ComposeResult compose(const ForeignIR& ir, const Research& research,
                      const std::string& run_id, const std::string& seed);

/* Build a complete Void Core state document from the composed mantles. */
std::string emit_document(const ComposeResult& composed, const ForeignIR& ir,
                          const std::string& run_id, const std::string& seed,
                          std::string& error);

/* Structural validation of the emitted document: unique names and ids, declared
 * glyphs, resolving links, measure quantities, an active mantle that exists. */
Validation validate_document(std::string_view document);

std::string residual_json(const ForeignIR& ir, const Research& research);
std::string research_markdown(const ForeignIR& ir, const Research& research,
                              const std::string& run_id);

/* The host's derived cache format. */
std::string research_to_json(const Research& research);
bool research_from_json(std::string_view text, Research& out, std::string& error);

/* The whole pure path in memory, for the unit test and the smoke test. */
bool run_slice(std::string_view ir_json, const std::string& run_id,
               const std::string& seed, Slice& out, std::string& error);

}  // namespace squid::molerat
