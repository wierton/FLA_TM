#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#define DEBUG

#ifdef DEBUG
#  define pdbg(fmt, ...)                              \
    std::cerr << formatv("%s: [%s:%s]" fmt, __LINE__, \
        wis.get_lineno(), wis.get_column(), ##__VA_ARGS__)
#else
#  define pdbg(fmt, ...)
#endif

namespace opt {
bool verbose = false;
};

class Tape {
  std::vector<char> p_tape; // 0, 1, 2, ...
  std::vector<char> n_tape; // -1, -2, ...
  char blank = '_';
  int64_t index = 0;

  char tape_at(int64_t i) const {
    /* emplace back blank */
    if (i >= 0) {
      if (i >= p_tape.size()) return '_';
      return p_tape.at(i);
    } else {
      if (-i - 1 >= n_tape.size()) return '_';
      return n_tape.at(-i - 1);
    }
  }

  char &tape_at(int64_t i) {
    /* emplace back blank */
    if (i >= 0) {
      while (p_tape.size() < i + 1) p_tape.push_back(blank);
      return p_tape.at(i);
    } else {
      while (n_tape.size() < -i) n_tape.push_back(blank);
      return n_tape.at(-i - 1);
    }
  }

public:
  Tape(char blank) : blank(blank) {}

  int64_t begin() const {
    return n_tape.size() ? (-n_tape.size()) : 0;
  }
  int64_t end() const { return p_tape.size(); }
  int64_t cbegin() const {
    int64_t l = begin();
    int64_t r = end();
    for (; l < r; l++)
      if (tape_at(l) != '_') break;
    return l;
  }
  int64_t cend() const {
    int64_t l = begin();
    int64_t r = end();
    for (; l < r; r--)
      if (tape_at(r - 1) != '_') break;
    return r;
  }

  size_t size() const { return p_tape.size(); }
  char get(int64_t i) const { return tape_at(i); }
  char get() { return tape_at(index); }
  int64_t get_index() const { return index; }
  void set(const std::string &s) {
    p_tape.clear();
    for (char ch : s) p_tape.push_back(ch);
  }

  void setAndMove(char ch, char dir) {
    tape_at(index) = ch;
    if (dir == 'l')
      index--;
    else if (dir == 'r')
      index++;
  }

  std::string get_contents() const {
    std::string ret;
    for (int64_t i = cbegin(); i < cend(); i++)
      ret.push_back(tape_at(i));
    return ret;
  }
};

bool operator<(const std::vector<char> &l,
    const std::vector<char> &r) {
  for (unsigned i = 0; i < l.size(); i++) {
    if (i >= r.size()) return false;
    if (l[i] != r[i]) return l[i] < r[i];
  }
  return l.size() < r.size();
}

class TuringMachine {
  std::vector<Tape> tapes;
  std::vector<std::string> stateStrings;

  unsigned state = 0u;
  char blank = '_';
  std::set<unsigned> finalStates;
  unsigned nr_steps = 0u;

  struct TransitionInfo {
    //                  nxtSym, action
    std::vector<std::pair<char, char>> nxtStep;
    unsigned nxtState;
  };

  // state -> symbol vec -> transition info
  std::vector<std::map<std::vector<char>, TransitionInfo>>
      delta;

  friend class TMParser;

public:
  TuringMachine(unsigned nTapes, char blank) {
    this->blank = blank;
    for (unsigned i = 0; i < nTapes; i++)
      tapes.emplace_back(blank);
  }

  void set_input(const std::string &s) {
    for (unsigned i = 0; i < s.size(); i++) { ; }
    tapes.at(0).set(s);
  }

  std::vector<char> getCurSymbols() {
    std::vector<char> ret;
    for (auto &tape : tapes) ret.push_back(tape.get());
    return ret;
  }

  bool isTerminated() const {
    return finalStates.find(state) != finalStates.end();
  }

  void runOneStep() {
    if (state >= delta.size()) {
      /* cannot proceed */
      std::cerr << "error: cannot proceed since no guidelines "
                   "about current state\n";
      exit(1);
    }

    auto &m = delta[state];
    auto symbols = getCurSymbols();

    auto it = m.find(symbols);
    if (it == m.end()) {
      /* cannot proceed */
      std::cerr << "error: cannot proceed since no guidelines "
                   "about current tape symbols\n";
      exit(1);
    }

    auto &info = it->second;
    auto &step = info.nxtStep;

    /* set new state */
#ifdef DEBUG
    assert(step.size() == tapes.size());
#endif
    for (unsigned i = 0; i < step.size(); i++)
      if (i < tapes.size())
        tapes[i].setAndMove(step[i].first, step[i].second);
    state = info.nxtState;
  }

  std::string run() {
    while (!isTerminated()) {
      if (opt::verbose) printOneStep();
      runOneStep();
      nr_steps++;
    }
    if (opt::verbose) printOneStep();

    if (tapes.empty()) return "";
    return tapes.at(0).get_contents();
  }

  void printOneStep() {
    /* Step   : 0
     * Index0 : 0 1 2 3 4 5 6
     * Tape0  : 1 0 0 1 0 0 1
     * Head0  : ^
     * Index1 : 0
     * Tape1  : _
     * Head1  : ^
     * State  : 0
     * ---------------------------------------------
     **/
    std::cout << "Step   : " << nr_steps << "\n";

    for (unsigned i = 0; i < tapes.size(); i++) {
      int64_t cbegin = tapes[i].cbegin();
      int64_t cend = tapes[i].cend();
      int64_t index = tapes[i].get_index();
      if (cbegin >= cend) {
        cbegin = index;
        cend = index + 1;
      } else if (index >= cend) {
        cend = index + 1;
      } else if (index < cbegin) {
        cbegin = index;
      }

      std::cout << "Index" << i << " :";
      for (int64_t j = cbegin; j < cend; j++)
        std::cout << " " << j;
      std::cout << "\n";

      std::cout << "Tape" << i << "  :";
      for (int64_t j = cbegin; j < cend; j++) {
        unsigned n = 1;
        if (j != cbegin)
          n = std::to_string(j - 1).size();
        for (unsigned i = 0; i < n; i ++)
          std::cout << " ";
        std::cout << tapes[i].get(j);
      }
      std::cout << "\n";

      std::cout << "Head" << i << "  :";
      for (int64_t j = cbegin; j < index; j++) {
        unsigned n = std::to_string(j).size();
        for (unsigned k = 0; k < n + 1; k ++)
          std::cout << " ";
      }
      std::cout << " ^\n";
    }

    std::cout << "State  : " << stateStrings.at(state)
              << "\n";
    /* clang-format off */
    std::cout << "---------------------------------------------\n";
    /* clang-format on */
  }

  void dump() {
    std::clog << "#Q = {";
    for (const std::string &s : stateStrings)
      std::clog << s << ", ";
    std::clog << "}\n";

    std::clog << "#q0 = " << stateStrings.at(state) << "\n";
    std::clog << "#B = " << blank << "\n";

    std::clog << "#F = {";
    for (unsigned s : finalStates)
      std::clog << stateStrings.at(s) << ", ";
    std::clog << "}\n";

    std::clog << "#N = " << tapes.size() << "\n";

    for (unsigned i = 0; i < delta.size(); i++) {
      auto &m = delta[i];
      for (auto &kvpair : m) {
        const std::vector<char> &symvec = kvpair.first;
        TransitionInfo &info = kvpair.second;

        std::clog << stateStrings[i] << " ";
        for (char ch : symvec) std::clog << ch;
        std::clog << " ";

        for (std::pair<char, char> chs : info.nxtStep)
          std::clog << chs.first;
        std::clog << " ";

        for (std::pair<char, char> chs : info.nxtStep)
          std::clog << chs.second;
        std::clog << " ";

        std::clog << stateStrings[info.nxtState] << "\n";
      }
    }
  }
};

template <class... Args>
std::string formatv(const char *fmt, Args &&... args) {
  std::vector<std::string> args_strs = {
      static_cast<std::ostringstream &>(
          std::ostringstream{}
          << std::forward<Args>((Args)args))
          .str()...};

  std::string ret;
  unsigned counter = 0;
  for (const char *p = fmt; *p; p++) {
    if (*p == '%') {
      p++;
      if (*p == 's')
        ret += args_strs.at(counter++);
      else if (*p != 0)
        ret += *p;
      else
        break;
    } else {
      ret += *p;
    }
  }
  return ret;
}

#if 0
template <class T>
class error_or : public std::optional<T> {
  std::ostringstream oss;

public:
  using std::optional<T>::optional<T>;
  using std::optional<T>::operator=;
  using std::optional<T>::operator bool;

  template <class U>
  friend error_or &operator<<(error_or &os, U &&msg) {
    oss << std::forward<U>(msg);
    return *this;
  }

  const std::string &get_error() const { return oss.str(); }
  std::string &&get_error() { return oss.str(); }

  bool has_error() {
    return !bool(static_cast<std::optional<T> &>(*this));
  }
};
#endif

class wrapped_istream {
  unsigned lineno = 0;
  unsigned column = 0;
  std::vector<std::string> lines;
  std::istream &is;

  void advance_cursor(char ch) {
    if (ch == '\n' || ch == ';') {
      lineno++;
      column = 0;
      lines.emplace_back();
      if (ch == ';')
        while (is.get() != '\n')
          ;
    } else {
      lines.back().push_back(ch);
      column++;
    }
  }

public:
  wrapped_istream(std::istream &is) : is(is) {
    lines.emplace_back();
  }

  bool endl() const {
    char ch = is.peek();
    if (ch == '\n' || ch == ';') return true;
    return !is.good();
  }

  bool good() { return is.good(); }

  char get() {
    char ch = is.get();
    advance_cursor(ch);
    return ch;
  }

  char peek() { return is.peek(); }

  void ignore() { advance_cursor(is.get()); }

  void getline(std::string &s) {
    std::getline(is, s);
    advance_cursor('\n');
  }

  unsigned get_lineno() const { return lineno; }
  unsigned get_column() const { return column; }
  const std::string &get_line(unsigned i) const {
    return lines.at(i);
  }
};

class TMParser {
  std::map<std::string, unsigned> stateIdMap;
  struct StringToken : public std::string {
    unsigned lineno = 0;
    unsigned column = 0;
    using std::string::string;
    using std::string::operator=;
  };

  // #Q = {0,cp,cmp,mh,accept}
  std::vector<StringToken> states;
  // #S = {0,1}
  std::vector<StringToken> inputSymbolSet; // #S
  // #G = {0,1,_,t,r,u,e,f,a,l,s}
  std::vector<StringToken> tapeSymbolSet; // #G
  // #q0 = 0
  StringToken initState;
  // #B = _
  StringToken blankSymbol; // #B
  // #F = {halt_accept}
  std::vector<StringToken> finalStates; // #F
  // #N = 2
  unsigned nTapes = -1u; // #N
  // cmp 01 __ rl reject
  // 0 __ __ ** accept

  struct DeltaEntry {
    StringToken curState;
    StringToken curSymbols;
    StringToken nxtSymbols;
    StringToken actions;
    StringToken nxtState;
  };
  std::vector<DeltaEntry> delta;

  static void report_error_here(
      const std::string &msg, wrapped_istream &wis) {
    StringToken s(" ");
    s.lineno = wis.get_lineno();
    s.column = wis.get_column();
    report_error(s, msg, wis);
  }

  static void report_error(const StringToken &tok,
      const std::string &msg, wrapped_istream &wis) {
    if (!opt::verbose) {
      std::cerr << "syntax error\n";
      exit(1);
    }

    std::cerr << "error at line " << (wis.get_lineno() + 1)
              << " column " << (wis.get_column() + 1)
              << ": " << msg << "\n";
    std::cerr << wis.get_line(tok.lineno);
    if (tok.lineno == wis.get_lineno()) {
      char ch = wis.peek();
      if (ch != '\n') std::cerr << ch;
    }
    std::cerr << '\n';
    for (unsigned i = 0; i < tok.column; i++)
      std::cerr << " ";
    for (unsigned i = 0;
         i < std::max<unsigned>(tok.size(), 1u); i++)
      std::cerr << "^";
    std::cerr << '\n';
  }

private:
  static void stringStrip(std::string &s) {
    s.erase(0, s.find_first_not_of(" \t\v\f"));
    s.erase(s.find_last_not_of(" \t\v\f"), s.npos);
  }

  static bool erase_blank(wrapped_istream &wis) {
    while (std::isblank(wis.peek()) && wis.good())
      wis.ignore();
    return wis.good();
  }

  static bool erase_blank_until(
      wrapped_istream &wis, char ch) {
    erase_blank(wis);
    if (wis.get() != ch) {
      report_error_here(
          formatv("expected `%s' here", ch), wis);
      return false;
    }
    return wis.good();
  }

  static StringToken parseStringToken(wrapped_istream &wis,
      std::function<bool(char)> tester) {
    StringToken token;
    token.lineno = wis.get_lineno();
    token.column = wis.get_column();
    while (!wis.endl()) {
      char ch = wis.peek();
      if (tester(ch)) {
        wis.ignore();
        token.push_back(ch);
      } else
        break;
    }
    return token;
  }

  static StringToken parseState(wrapped_istream &wis) {
    StringToken state = parseStringToken(wis, [](char ch) {
      return std::isalnum(ch) || ch == '_';
    });

    if (state.size() == 0)
      report_error_here("expected state here", wis);
    return state;
  }

  static StringToken parseActions(wrapped_istream &wis) {
    StringToken symbol = parseStringToken(wis, [](char ch) {
      return ch == 'l' || ch == 'r' || ch == '*';
    });

    if (symbol.size() == 0)
      report_error_here("expected input symbol here", wis);
    return symbol;
  }

  static StringToken parseInputSymbol(
      wrapped_istream &wis) {
    StringToken symbol = parseStringToken(wis, [](char ch) {
      return std::isprint(ch) && ch != ' ' && ch != ',' &&
             ch != ';' && ch != '{' && ch != '}' &&
             ch != '*' && ch != '_';
    });

    if (symbol.size() == 0)
      report_error_here("expected input symbol here", wis);
    return symbol;
  }

  static StringToken parseTapeSymbol(wrapped_istream &wis) {
    StringToken symbol = parseStringToken(wis, [](char ch) {
      return std::isprint(ch) && ch != ' ' && ch != ',' &&
             ch != ';' && ch != '{' && ch != '}' &&
             ch != '*';
    });

    if (symbol.size() == 0)
      report_error_here("expected tape symbol here", wis);
    return symbol;
  }

  static std::vector<StringToken> parseStringArray(
      wrapped_istream &wis,
      std::function<StringToken(wrapped_istream &)>
          extractor) {
    pdbg("[parseStringArray] erase blank\n");
    erase_blank(wis);
    pdbg("[parseStringArray] after erase blank, '%s'\n",
        wis.peek());
    if (wis.get() != '{')
      report_error_here(
          "expected '{' here, parse it anyway", wis);

    std::vector<StringToken> retSet;
    while (!wis.endl()) {
      StringToken s = extractor(wis);
      pdbg(
          "[parseStringArray] extract '%s'-'%s' "
          "[%s:%s:%s]\n",
          std::string(s), wis.peek(), s.lineno, s.column,
          s.size());
      if (s.size() == 0) { continue; }
      retSet.push_back(s);
      erase_blank(wis);
      if (wis.endl()) break;

      char ch = wis.get();
      if (ch == '}')
        break;
      else if (ch == ',')
        continue;
      else
        report_error_here("expected '}' or ',' here", wis);
    }

    erase_blank(wis);
    pdbg("[parseStringArray] final blank '%s'\n",
        wis.peek());
    return retSet;
  }

  static std::vector<StringToken> parseStateArray(
      wrapped_istream &wis) {
    return parseStringArray(wis, &parseState);
  }

  static std::vector<StringToken> parseInputSymbolArray(
      wrapped_istream &wis) {
    return parseStringArray(wis, &parseInputSymbol);
  }

  static std::vector<StringToken> parseTapeSymbolArray(
      wrapped_istream &wis) {
    return parseStringArray(wis, &parseTapeSymbol);
  }

public:
  TMParser() {}

  bool validate_input(const std::string &input) {
    /* ERROR
     *
     * Input: 100A1A001
     * ==================== ERR ====================
     * error: 'A' was not declared in the set of input
     * symbols Input: 100A1A001
     *  		 ^
     * ==================== END ====================
     *
     * CORRECT
     * Input: 1001001
     * ==================== RUN ====================
     * */
    std::set<char> valid_chars;
    for (auto &s : inputSymbolSet)
      valid_chars.insert(s.at(0));

    for (unsigned i = 0; i < input.size(); i++) {
      char ch = input[i];
      if (valid_chars.find(ch) != valid_chars.end())
        continue;

      if (opt::verbose) {
        /* clang-format off */
        std::cerr << "Input: " << input << "\n";
        std::cerr << "==================== ERR ====================\n";
        std::cerr << "error: '" << ch
                  << "' was not declared in the set of input symbols\n";
        std::cerr << "Input: " << input << "\n";
        for (unsigned j = 0; j < 7 + i; j++) std::cerr << " ";
        std::cerr << "^\n";
        std::cerr << "==================== END ====================\n";
        /* clang-format on */
      } else {
        std::cerr << "illegal input\n";
      }
      return false;
    }
    return true;
  }

  TuringMachine parseTMFile(std::istream &is) {
    wrapped_istream wis(is);
    // a naive parser
    unsigned preseted_nTapes = -1;
    while (wis.good()) {
      pdbg("[mainloop] of parser's main while loop\n");
      erase_blank(wis);
      pdbg("[mainloop] erase initial blank, '%s'\n",
          wis.peek());

      if (wis.endl()) {
        pdbg(
            "[mainloop.endl] meet endl '%s'\n", wis.peek());
        wis.get();
      } else if (wis.peek() == '#') {
        wis.ignore();
        pdbg("[mainloop.#] erase blank\n");
        erase_blank(wis);
        pdbg("[mainloop.#] after erase blank '%s'\n",
            wis.peek());
        char ch = wis.get();
        switch (ch) {
        case 'Q':
        case 'S':
        case 'G':
        case 'F': {
          erase_blank_until(wis, '=');
          pdbg("[mainloop.#|=] after erase blank '%s'\n",
              wis.peek());
          std::vector<StringToken> sv =
              (ch == 'Q' || ch == 'F')
                  ? parseStateArray(wis)
                  : (ch == 'S' ? parseInputSymbolArray(wis)
                               : parseTapeSymbolArray(wis));

          if (ch == 'Q') {
            states = std::move(sv);
            unsigned counter = 0;
            for (const StringToken &state : states)
              stateIdMap[state] = counter++;
          } else if (ch == 'F') {
            finalStates = std::move(sv);
          } else if (ch == 'S') {
            for (const StringToken &symbol : sv) {
              if (symbol.size() > 1) {
                report_error(symbol,
                    "expected only one character as symbol "
                    "in #S",
                    wis);
              }
            }
            inputSymbolSet = std::move(sv);
          } else if (ch == 'G') {
            for (const StringToken &symbol : sv) {
              if (symbol.size() > 1) {
                report_error(symbol,
                    "expected only one character as symbol "
                    "in #G",
                    wis);
              }
            }
            tapeSymbolSet = std::move(sv);
          }
        } break;
        case 'q':
          if ((ch = wis.get()) != '0') {
            report_error_here(
                formatv("expected #q0 here, not #q%s", ch),
                wis);
          }
          erase_blank_until(wis, '=');
          erase_blank(wis);
          pdbg("[mainloop.#q0] next '%s'\n", wis.peek());
          initState = parseState(wis);
          break;
        case 'B':
          erase_blank_until(wis, '=');
          erase_blank(wis);
          blankSymbol = parseState(wis);
          break;
        case 'N':
          erase_blank_until(wis, '=');
          erase_blank(wis);
          char ch;
          unsigned n = 0;
          while (std::isdigit(ch = wis.get()))
            n = (ch - '0') + n * 10;
          if (nTapes != -1u) {
            report_error_here(
                formatv("#N has been deduced to be %s from "
                        "delta function",
                    nTapes),
                wis);
          }
          nTapes = n;
          preseted_nTapes = n;
          break;
        }

        while (!wis.endl()) wis.ignore();
        wis.ignore();
      } else {
        pdbg("[mainloop.action], next '%s'\n", wis.peek());
        DeltaEntry e;
        e.curState = parseState(wis);
        pdbg("[mainloop.action.State] '%s'\n",
            std::string(e.curState));
        erase_blank(wis);
        e.curSymbols = parseTapeSymbol(wis);
        pdbg("[mainloop.action.CurSym] '%s'\n",
            std::string(e.curSymbols));
        erase_blank(wis);
        e.nxtSymbols = parseTapeSymbol(wis);
        pdbg("[mainloop.action.NxtSym] '%s'\n",
            std::string(e.nxtSymbols));
        erase_blank(wis);
        e.actions = parseActions(wis);
        pdbg("[mainloop.action.Actions] '%s'\n",
            std::string(e.actions));
        erase_blank(wis);
        e.nxtState = parseState(wis);
        while (!wis.endl()) wis.ignore();
        wis.ignore();
        delta.push_back(e);

        const std::vector<const StringToken *> sv = {
            &e.curSymbols, &e.nxtSymbols, &e.actions};
        for (const StringToken *stp : sv) {
          unsigned old = nTapes;
          nTapes = std::min<unsigned>(stp->size(), nTapes);
          if (preseted_nTapes != -1u &&
              preseted_nTapes != stp->size()) {
            report_error(e.curSymbols,
                formatv(
                    "#N: %s, size %s here is inconsistent, "
                    "we adjust #N to %s as minimum value",
                    preseted_nTapes, stp->size(), nTapes),
                wis);
          } else if (nTapes != -1 &&
                     nTapes != stp->size()) {
            report_error(e.curSymbols,
                formatv(
                    "deduced #N: %s, size %s here is "
                    "inconsistent, "
                    "we adjust #N to %s as minimum value",
                    old, stp->size(), nTapes),
                wis);
          }
        }

        for (unsigned i = 0; i < e.actions.size(); i++) {
          char a = e.actions[i];
          if (a != 'l' && a != 'r' && a != '*') {
            StringToken tok(" ");
            tok.lineno = wis.get_lineno();
            tok.column = wis.get_column() + i;
            report_error(tok, "expected l, r, * here", wis);
          }
        }
      }
    }
    /* check state */
    for (const StringToken &s : finalStates) {
      if (stateIdMap.find(s) == stateIdMap.end())
        report_error(s,
            formatv("cannot find state '%s' in #Q from #F",
                std::string(s)),
            wis);
    }
    for (const DeltaEntry &e : delta) {
      if (stateIdMap.find(e.nxtState) == stateIdMap.end())
        report_error(e.nxtState,
            formatv("cannot find state '%s' in actions",
                std::string(e.nxtState)),
            wis);
    }

    if (stateIdMap.find(initState) == stateIdMap.end())
      report_error(initState,
          formatv("cannot find state %s (=#q0) in #Q",
              std::string(initState)),
          wis);

    if (nTapes == -1u) {
      std::cerr << "invalid #N " << nTapes << "\n";
      nTapes = 0u;
    }

    /* construct TuringMachine */
    TuringMachine TM(nTapes, blankSymbol[0]);
    TM.state = stateIdMap[initState];
    for (const StringToken &s : finalStates) {
      TM.finalStates.insert(stateIdMap[s]);
    }

    for (const StringToken &s : states)
      TM.stateStrings.emplace_back(s);

    for (const DeltaEntry &e : delta) {
      unsigned cur_state = stateIdMap[e.curState];
      TM.delta.resize(std::max<unsigned>(
          TM.delta.size(), cur_state + 1));
      std::vector<char> cur_symvec(e.curSymbols.begin(),
          e.curSymbols.begin() + nTapes);
      std::vector<std::pair<char, char>> nxtSym_action_vec;
      for (unsigned i = 0; i < nTapes; i++)
        nxtSym_action_vec.emplace_back(
            e.nxtSymbols[i], e.actions[i]);

      auto &info = TM.delta[cur_state][cur_symvec];
      info.nxtStep = std::move(nxtSym_action_vec);
      info.nxtState = stateIdMap[e.nxtState];
    }
    return TM;
  }
};

int main(int argc, const char *argv[]) {
  const char *help =
      "usage: turing [-v|--verbose] [-h|--help] <tm> "
      "<input>";
  if (argc <= 1) {
    std::cout << help << "\n";
    return 1;
  }

  const char *tmfile = nullptr;
  const char *input = nullptr;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 ||
        strcmp(argv[i], "--help") == 0) {
      std::cout << help << "\n";
      return 0;
    } else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0) {
      opt::verbose = 1;
    } else if (!tmfile) {
      tmfile = argv[i];
    } else if (!input) {
      input = argv[i];
    } else {
      std::cerr << "invalid argument '" << argv[i] << "'\n";
      return 0;
    }
  }

  if (!tmfile || !input) {
    std::cout << help << "\n";
    return 1;
  }

  std::ifstream ifs(tmfile);
  TMParser parser;
  auto TM = parser.parseTMFile(ifs);
  if (!parser.validate_input(input)) {
    return 1;
  }

  if (opt::verbose) {
    /* clang-format off */
    std::cout << "Input: " << input << "\n";
    std::cout << "==================== RUN ====================\n";
    /* clang-format on */
  }

  TM.set_input(input);
#ifdef DEBUG
  TM.dump();
#endif
  std::string result = TM.run();
  if (opt::verbose) {
    /* clang-format off */
    std::cout << "Result: " << result << "\n";
    std::cout << "==================== END ====================\n";
    /* clang-format on */
  } else {
    std::cout << result << "\n";
  }
  return 0;
}
