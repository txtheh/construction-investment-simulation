#pragma once
#include <string>
#include <vector>

class Decision {
public:
    Decision(int id, int month, std::string decision, double budget);
    ~Decision();

    int getIdPlayer() const;
    int getMonth() const;
    std::string getTypeDecision() const;
    double getBudget() const;

    // Возврат порядка строительства
    // std::vector<Building> getBuildOrder();

    void setIdPlayer(int id);
    void setMonth(int month);
    void setTypeDecision(std::string decision);
    void setBudget(double budget);


private:
    int idPlayer_;
    int month_;
    std::string type_decision_;

    double budget_;

    // реализация порядка строительства
    // std::vector<Building*> buildOrder_;
    // std::map ...
}
