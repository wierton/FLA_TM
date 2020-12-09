#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

class Tape {
  std::vector<char> tape;
  unsigned index;

public:
  Tape(unsigned index = 0) : index(index) {}

  char get() { return tape.at(index); }

  void setAndMove(char ch, char dir) {
    tape.at(index) = ch;
    if (dir == 'l')
      index--;
    else
      index++;
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
  char blank = 0u;
  std::set<unsigned> finalStates;

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
  TuringMachine(unsigned nTapes) : tapes(nTapes) {}

  std::vector<char> getCurSymbols() {
    std::vector<char> ret;
    for (auto &tape : tapes) ret.push_back(tape.get());
    return ret;
  }

  bool isTerminated() const {
    return finalStates.find(state) != finalStates.end();
  }

  void runOneStep() {
    assert(state < delta.size());
    auto &m = delta[state];
    auto symbols = getCurSymbols();

    auto it = m.find(symbols);
    assert(it != m.end());

    auto &info = it->second;
    auto &step = info.nxtStep;

    /* set new state */
    assert(step.size() == tapes.size());
    for (unsigned i = 0; i < step.size(); i++)
      tapes[i].setAndMove(step[i].first, step[i].second);
    state = info.nxtState;
  }

  void run() {
    while (!isTerminated()) runOneStep();
  }

  void printOneStep() {}
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
    if (ch == '\n') {
      lineno++;
      column = 0;
      lines.emplace_back();
    } else {
      lines.back().push_back(ch);
      column++;
    }
  }

public:
  wrapped_istream(std::istream &is) : is(is) {
    lines.emplace_back();
  }

  bool meet_endl() const {
    char ch = is.peek();
    if (ch == '\n') return true;
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
    std::cerr << "error at line " << (wis.get_lineno() + 1)
              << " column " << (wis.get_column() + 1)
              << ": " << msg << "\n";
    unsigned col = wis.get_column();
    std::cerr << wis.get_line(tok.lineno);
    if (tok.lineno == wis.get_lineno()) {
      char ch = wis.peek();
      if (ch != '\n') std::cerr << ch;
    }
    std::cerr << '\n';
    for (unsigned i = 0; i < col; i++) std::cerr << " ";
    for (unsigned i = 0; i < tok.size(); i++)
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

  static StringToken parseState(wrapped_istream &wis) {
    StringToken state;
    while (!wis.meet_endl()) {
      char ch = wis.peek();
      if (std::isalnum(ch) || ch == '_') {
        wis.ignore();
        state.push_back(ch);
      } else
        break;
    }

    if (state.size() == 0)
      report_error_here("expected state here", wis);
    return state;
  }

  static StringToken parseSymbol(wrapped_istream &wis) {
    StringToken symbol;
    while (!wis.meet_endl()) {
      char ch = wis.peek();
      if (std::isprint(ch) && ch != ' ' && ch != ',' &&
          ch != ';' && ch != '*' && ch != '_') {
        wis.ignore();
        symbol.push_back(ch);
      } else
        break;
    }

    if (symbol.size() == 0)
      report_error_here("expected symbol here", wis);
    return symbol;
  }

  static std::vector<StringToken> parseStringArray(
      wrapped_istream &wis,
      std::function<StringToken(wrapped_istream &)>
          extractor) {
    erase_blank(wis);
    if (wis.get() != '{')
      report_error_here(
          "expected '{' here, parse it anyway", wis);

    std::vector<StringToken> retSet;
    while (!wis.meet_endl()) {
      StringToken s = extractor(wis);
      if (s.size() == 0) { continue; }
      retSet.push_back(s);
      bool safe = erase_blank(wis);
      if (!safe) break;

      char ch = wis.get();
      if (ch == '}')
        break;
      else if (ch == ',')
        continue;
      else
        report_error_here("expected '}' or ',' here", wis);
    }

    erase_blank(wis);
    return retSet;
  }

  static std::vector<StringToken> parseStateArray(
      wrapped_istream &wis) {
    return parseStringArray(wis, &parseState);
  }

  static std::vector<StringToken> parseSymbolArray(
      wrapped_istream &wis) {
    return parseStringArray(wis, &parseSymbol);
  }

public:
  TMParser() {}

  TuringMachine parseTMFile(wrapped_istream &wis) {
    // a naive parser
    unsigned preseted_nTapes = -1;
    while (wis.good()) {
      erase_blank(wis);

      if (wis.get() == '#') {
        erase_blank(wis);
        char ch = wis.get();
        switch (ch) {
        case 'Q':
        case 'S':
        case 'G':
        case 'F': {
          erase_blank_until(wis, '=');
          std::vector<StringToken> sv =
              (ch == 'Q' || ch == 'F')
                  ? parseStateArray(wis)
                  : parseSymbolArray(wis);

          if (ch == 'Q') {
            states = std::move(sv);
            unsigned counter = 0;
            for (const StringToken &state : sv)
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

        while (!wis.meet_endl()) wis.ignore();
        wis.ignore();
      } else {
        DeltaEntry e;
        e.curState = parseState(wis);
        erase_blank(wis);
        e.curSymbols = parseSymbol(wis);
        erase_blank(wis);
        e.nxtSymbols = parseSymbol(wis);
        erase_blank(wis);
        e.actions = parseState(wis);
        erase_blank(wis);
        e.nxtState = parseState(wis);
        while (!wis.meet_endl()) wis.ignore();
        wis.ignore();

        const std::vector<const StringToken *> sv = {
            &e.curSymbols, &e.nxtSymbols, &e.actions};
        for (const StringToken *stp : sv) {
          unsigned old = nTapes;
          nTapes = std::min((unsigned)stp->size(), nTapes);
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
            formatv("cannot find state %s in #Q",
                std::string(s)),
            wis);
    }
    for (const DeltaEntry &e : delta) {
      if (stateIdMap.find(e.nxtState) == stateIdMap.end())
        report_error(e.nxtState,
            formatv("cannot find state %s in #Q",
                std::string(e.nxtState)),
            wis);
    }

    if (stateIdMap.find(initState) == stateIdMap.end())
      report_error(initState,
          formatv(
              "cannot find state %s in #Q", std::string(initState)),
          wis);

    if (nTapes == -1u) {
      std::cerr << "invalid #N " << nTapes << "\n";
      nTapes = 0u;
    }

    /* construct TuringMachine */
    TuringMachine TM(nTapes);
    TM.state = stateIdMap[initState];
    for (const StringToken &s : finalStates) {
      TM.finalStates.insert(stateIdMap[s]);
      TM.blank = blankSymbol[0];
      for (const DeltaEntry &e : delta) {
        unsigned cur_state = stateIdMap[e.curState];
        TM.delta.resize(std::max(
            (unsigned)TM.delta.size(), cur_state + 1));
        std::vector<char> cur_symvec(e.curSymbols.begin(),
            e.curSymbols.begin() + nTapes);
        std::vector<std::pair<char, char>>
            nxtSym_action_vec;
        for (unsigned i = 0; i < nTapes; i++)
          nxtSym_action_vec.emplace_back(
              e.nxtSymbols[i], e.actions[i]);

        auto &info = TM.delta[cur_state][cur_symvec];
        info.nxtStep = std::move(nxtSym_action_vec);
        info.nxtState = stateIdMap[e.nxtState];
      }
    }
    return TM;
  }
};

int main() { return 0; }
