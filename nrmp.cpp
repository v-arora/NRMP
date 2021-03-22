#include "nrmp.h"
#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>
#include <unordered_set>
using namespace std;

  Program::Program(const string &name, const string &file_path) : name_(name) {
    parseFile(file_path);
  }

  bool Program::canMatchCandidate(const string& str, string* displaced) {
    auto it = ranking_.find(str);
    if (it != ranking_.end()) {
      printf("%s can check for %s\n", name_.c_str(), str.c_str());
      int priority = it->second;

      if (free_positions_) {
        positions_[priority] = str;
        --free_positions_;
        printf("%s %d got a free seat at %s\n", str.c_str(), priority, name_.c_str());
        return true;
      } else {
        for (auto itr = positions_.rbegin(); itr != positions_.rend(); ++itr) {
          int filled_priority = itr->first;
          if (priority < filled_priority) {
            *displaced = itr->second;
            printf("%s %d is replacing %s %d at %s\n", str.c_str(), priority, displaced->c_str(), filled_priority, name_.c_str());
            positions_.erase(itr->first);
            positions_[priority] = str;
            return true;
          }
        }
      }
    } else {
      printf("did not find %s in %s\n", str.c_str(), name_.c_str());
    }
    return false;
  }
  void Program::getPositions(vector<string> *pos) {

  }
  void Program::dump() {
    printf("name: %s\n", name_.c_str());
    for (auto &info : ranking_) {
      const string& candidate = info.first;
      const int &pri = info.second;
      printf("%s -> %d\n", candidate.c_str(), pri);
    }
  }

  void Program::parseFile(const string &file_path) {
    ifstream istream(file_path);
    if (istream.is_open()) {
      string line {};
      while (getline(istream, line)) {
        auto found = line.find(',');
        if (found != string::npos) {
          string key = line.substr(0, found);
          int value = stoi(line.substr(found+1, string::npos));
          if (key == "positions") {
            printf("%s has %d positions\n", name_.c_str(), value);
            free_positions_ = value;
          } else {
            ranking_[key] = value;
            printf("Saving %s + %d in %s\n", key.c_str(), value, name_.c_str());
          }
        }
      }
    }
  }

  Candidate::Candidate(const string& name, const string &file_path) : name_(name) {
    parseFile(file_path);
  }

  void Candidate::parseFile(const string& file_path) {
    ifstream istream(file_path);
    if (istream.is_open()) {
      string line {};
      while (getline(istream, line)) {
        auto found = line.find(',');
        if (found != string::npos) {
          string program = line.substr(0, found);
          int priority = stoi(line.substr(found+1, string::npos));
          ranking_[priority] = program;
        }
      }
    }
  }

  string Candidate::getName() {
    return name_;
  }

  string Candidate::getNextProgram() {
    string program = "";
    if (target_program_number_ <= ranking_.size()) {
      auto it = ranking_.find(target_program_number_);
      if (it != ranking_.end()) {
        program = it->second;
        printf("%s next program: %s\n", name_.c_str(), program.c_str());
      }
    }

    ++target_program_number_;
    return program;
  }

  string Candidate::getAssignedProgram() {

    return "";
  }
  void Candidate::setProgram(const string& str) {
    program_ = str;
  }

  void Candidate::resetPrograms() {
    target_program_number_ = 1;
  }

  void Candidate::dump() {
    printf("name: %s, program: %s\n", name_.c_str(), program_.c_str());
    for (auto &info : ranking_) {
      const int &pri = info.first;
      string& program = info.second;
      printf("\t%d -> %s\n", pri, program.c_str());
    }
  }

  void NRMP::init() {
    createCandidates();
    createPrograms();
    printCandidates();
    printPrograms();
  }

  void NRMP::createCandidates() {
    candidates_ = {};

    unordered_map<string, string> names_to_files {};
    getCandidates(&names_to_files);

    for (auto& info : names_to_files) {
      const string &name = info.first;
      const string &path = info.second;

      candidates_[name] = new Candidate(name, path);
    }
  }
  void NRMP::getCandidates(unordered_map<string, string> *ntf) {
    getItems("./candidates", ntf);
  }
  void NRMP::createPrograms() {
    programs_ = {};

    unordered_map<string, string> names_to_files {};
    getPrograms(&names_to_files);

    for (auto& info : names_to_files) {
      const string &name = info.first;
      const string &path = info.second;

      programs_[name] = new Program(name, path);
    }
  }

  void NRMP::getPrograms(unordered_map<string, string> *ntf) {
    getItems("./programs", ntf);
  }

  void NRMP::getItems(const string& path, unordered_map<string, string> *ntf) {
    for (const auto &entry : filesystem::directory_iterator(path)) {
      const string &file_path = entry.path();
      smatch sm {};
      string exp = path + "\/(.*).txt";
      regex e(exp);
      regex_match(file_path, sm, e);
      if (sm.size()) {
        string name = sm[1];
        (*ntf)[name] = file_path;
      }
    }
  }

  void NRMP::match() {
    unordered_map<string, Candidate*> displaced {};

    // first pass
    for (auto &candidateInfo : candidates_) {
      const string &candidate_name = candidateInfo.first;
      Candidate *candidate_obj = candidateInfo.second;
      printf("Checking %s\n", candidate_name.c_str());
      string next_program = candidate_obj->getNextProgram();
      while (!next_program.empty()) {
        auto program_it = programs_.find(next_program);
        if (program_it != programs_.end()) {
          const string& program_name = program_it->first;
          Program* program_obj = program_it->second;
          string displaced_candidate {};
          printf("Checking %s in %s program\n", candidate_name.c_str(), program_name.c_str());
          if (program_obj->canMatchCandidate(candidate_name, &displaced_candidate)) {
            candidate_obj->setProgram(program_name);
            displaced.erase(candidate_name);
            if (!displaced_candidate.empty()) {
              displaced[displaced_candidate] = candidates_[displaced_candidate];
              candidates_[displaced_candidate]->setProgram("");
            }
            next_program = "";
          } else {
            next_program = candidate_obj->getNextProgram();
          }
        }
      }
    }
    printCandidates();

    // continue matching until displacements resolved
    while (!displaced.empty()) {
      for (auto &candidateInfo : displaced) {
        const string &candidate_name = candidateInfo.first;
        Candidate *candidate_obj = candidateInfo.second;
        candidate_obj->resetPrograms();
        printf("Checking %s\n", candidate_name.c_str());
        string next_program = candidate_obj->getNextProgram();
        while (!next_program.empty()) {
          auto program_it = programs_.find(next_program);
          if (program_it != programs_.end()) {
            const string& program_name = program_it->first;
            Program* program_obj = program_it->second;
            string displaced_candidate {};
            printf("Checking %s in %s program\n", candidate_name.c_str(), program_name.c_str());
            if (program_obj->canMatchCandidate(candidate_name, &displaced_candidate)) {
              candidate_obj->setProgram(program_name);
              if (!displaced_candidate.empty()) {
                displaced[displaced_candidate] = candidates_[displaced_candidate];
                candidates_[displaced_candidate]->setProgram("");
              }
              next_program = "";
            } else {
              next_program = candidate_obj->getNextProgram();
            }
          }
        }
        displaced.erase(candidate_name);
      }
      printCandidates();
    }
  }

  void NRMP::printPrograms() {
    for (auto& info : programs_) {
      Program *pr = info.second;
      pr->dump();
    }
    printf("\n");
  }
  void NRMP::printCandidates() {
    for (auto& info : candidates_) {
      Candidate *cd = info.second;
      cd->dump();
      printf("\n");
    }
  }


  int main(int argc, char **argv) {
    NRMP nrmp {};
    nrmp.init();
    nrmp.match();

    return 0;
  }