#pragma once
#include <string>

class Decision {
public:
    Decision(int id, int month, std::string decision,
            double housingAdBudget, double supermarketAdBudget);
    ~Decision();

    int getIdPlayer() const;
    int getMonth() const;
    std::string getTypeDecision() const;
    
    double getHousingAdBudget() const;
    double getSupermarketAdBudget() const;

    // Возврат порядка строительства
    // std::vector<Building> getBuildOrder();

    void setIdPlayer(int id);
    void setMonth(int month);
    void setTypeDecision(std::string decision);

private:
    int idPlayer_;
    int month_;
    std::string type_decision_;

    double housingAdBudget_;
    double supermarketAdBudget_;
};
