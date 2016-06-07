#include <iostream>
#include <string>
#include <regex>
#include <map>

#include <cassert>

typedef std::string::const_iterator Iterator;

struct KW {
    std::string kw;
    int64_t id;
    bool def;
    std::vector<KW> value;
    std::array<int64_t, 3> ctrs;
    std::string data;
    double fvalue;
};

struct Frame {
    int64_t level;
    std::vector<KW> value;
};

std::pair<bool, std::array<int64_t, 3>> parse_ctrs(Iterator begin, Iterator end, int base) {
    std::array<int64_t, 3> out;
    static std::regex ctr(":\\(([0-9a-f]+),([0-9a-f]+),([0-9a-f]+)\\)");
    std::smatch sm;
    auto ok = std::regex_match(begin, end, sm, ctr);
    if (!ok) return std::make_pair(false, out);
    
    for (unsigned i = 0; i < out.size(); i++) {
        out[i] = std::stoul(sm[i+1].str(), nullptr, base);
    }
    return std::make_pair(true, out);
}

std::pair<bool, KW> parse_kw(Iterator& begin, Iterator end, int base) {
    KW out;
    out.fvalue = 0.0;
    //                     1:KW     2:id     3:data 4           5:sub   6:num 7
    static std::regex kw("([A-Z]+)([0-9a-f]*)(=\\(([^ )=]*)\\))?(=\\()?(=([0-9.-]+))?.*");
    static std::regex tail("((:\\([^)]+\\))?([^ ()=:]*)\\s*).*");
    std::smatch sm;
    auto ok = std::regex_match(begin, end, sm, kw);
    if (!ok) return std::make_pair(false, out);
    out.kw = sm[1].str();
    out.id = sm[2].length() > 0 ? std::stoul(sm[2].str(), nullptr, base) : 0;
    out.def = true;
    if (sm[5].length() > 0) {
        begin = sm[5].second;
        for (;;) {
            auto kw = parse_kw(begin, end, base);
            if (kw.first) out.value.push_back(kw.second);
            if (!kw.first) break;
        }
        if (begin == end || *begin != ')') return std::make_pair(false, out);
        ++begin;
    } else if (sm[6].length() > 0) {
        out.fvalue = std::stof(sm[7].str());
        begin = sm[6].second;
    } else if (sm[3].length() > 0) {
        out.data = sm[4].str();
        begin = sm[3].second;
    } else {
        out.def = false;
        begin = sm[2].second;
    }
        
    ok = std::regex_match(begin, end, sm, tail);
    if (!ok) return std::make_pair(false, out);
    if (sm[2].length() > 0) {
        auto ctr = parse_ctrs(sm[2].first, sm[2].second, base);
        if (!ctr.first) return std::make_pair(false, out);
        out.ctrs = ctr.second;
    }
    if (sm[3].length() > 0) out.data = sm[3].str();
    begin = sm[1].second;
    return std::make_pair(true, out);
}

std::pair<bool, Frame> parse_frame(Iterator& begin, Iterator end, int base) {
    Frame out;
    static std::regex stmt("(C([0-9a-f]+)\\s+).*");
    std::smatch sm;
    auto ok = std::regex_match(begin, end, sm, stmt);
    if (!ok) return std::make_pair(false, out);
    out.level = std::stoul(sm[2].str(), nullptr, base);
    begin = sm[1].second;
    for (;;) {
        auto kw = parse_kw(begin, end, base);
        if (kw.first) out.value.push_back(kw.second);
        if (begin == end) return std::make_pair(true, out);
        if (!kw.first) return std::make_pair(false, out);
    }
}
    
int main() {
    std::string head;
    std::getline(std::cin, head);
    Iterator h_it = head.begin();
    auto info = parse_kw(h_it, head.end(), 10);
    int base = 10;
    if (!info.first || !info.second.def) exit(1);
    if (info.second.value.at(0).kw == "HEX") base = 16;
   
    std::vector<std::string*> stack;
    stack.resize(1024);
    std::map<int, KW> fns;
   
    while (std::cin) {
        std::string line;
        std::getline(std::cin, line);
        Iterator it = line.begin();
        auto res = parse_frame(it, line.end(), base);
        if (!res.first) {
            std::cerr << "Parse error: " << std::string(line.cbegin(), it) << "_" << std::string(it, line.cend()) << "\n";
            continue;
        }
        
        Frame& c = res.second;
        assert(c.value.size() >= 1);
        assert(c.value[0].kw == "FN");
        KW& fn = c.value[0];
        if (fn.def) {
            assert(fn.value.size() == 2);
            assert(fn.value[1].kw == "N");
            fns[fn.id] = fn;
        } 
        KW& fref = fns[fn.id];
        fref.data = fn.data;
        
        assert(c.level < stack.size());
        stack[c.level] = &fref.value[1].data;
        
        if (c.value.size() >= 2) {
            KW& ctr = c.value[1];
            assert(ctr.kw == "V");
            
            for (unsigned i = 1; i <= c.level; i++) {
                std::cout << *stack[i] << ";";
            }
            std::cout << " " << ctr.ctrs[0] << "\n";
        }
        
    }
    return 0;
}
