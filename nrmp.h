#include <string>
#include <unordered_map>
#include <map>
#include <vector>


using namespace std;

class Program;
class Candidate;

class Program {
  public:
    Program(const string& name, const string &file_path);
    bool canMatchCandidate(const string& str, string* displaced);
    void getPositions(vector<string> *pos);
    void dump();
  private:
    void parseFile(const string& file_path);
    string name_ {};
    unordered_map<string, int> ranking_ {};
    map<int, string> positions_ {};
    int free_positions_ = 0;
};

class Candidate {
  public:
    Candidate(const string& name, const string &file_path);
    string getName();
    string getNextProgram();
    string getAssignedProgram();
    void setProgram(const string& str);
    void resetPrograms();
    void dump();
  private:
    void parseFile(const string& file_path);
    const string name_ {};
    map<int, string> ranking_ {};
    int target_program_number_ = 1;
    string program_ {};
};

class NRMP {
  public:
    void init();
    void match();
    void printMatches();
  private:
    void createCandidates();
    void getCandidates(unordered_map<string, string> *ntf);
    void createPrograms();
    void getPrograms(unordered_map<string, string> *ntf);
    void printPrograms();
    void printCandidates();
    void getItems(const string &path, unordered_map<string, string> *ntf);
    unordered_map<string, Candidate*> candidates_ {};
    unordered_map<string, Program*> programs_ {};
};