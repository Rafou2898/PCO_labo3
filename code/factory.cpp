#include "factory.h"
#include "extractor.h"
#include "costs.h"
#include "wholesale.h"
#include <pcosynchro/pcothread.h>
#include <iostream>

WindowInterface* Factory::interface = nullptr;


Factory::Factory(int uniqueId, int fund, ItemType builtItem, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), resourcesNeeded(resourcesNeeded), itemBuilt(builtItem), nbBuild(0)
{
    assert(builtItem == ItemType::Chip ||
           builtItem == ItemType::Plastic ||
           builtItem == ItemType::Robot);

    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Factory created");
}

void Factory::setWholesalers(std::vector<Wholesale *> wholesalers) {
    Factory::wholesalers = wholesalers;

    for(Seller* seller: wholesalers){
        interface->setLink(uniqueId, seller->getUniqueId());
    }
}

ItemType Factory::getItemBuilt() {
    return itemBuilt;
}

int Factory::getMaterialCost() {
    return getCostPerUnit(itemBuilt);
}

bool Factory::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] == 0) {
            return false;
        }
    }

    return true;
}

void Factory::buildItem() {

    int builderCost = getEmployeeSalary(getEmployeeThatProduces(itemBuilt));

    mutex.lock();
    money -= builderCost;
    for (auto& item : resourcesNeeded) {
        --stocks[item];
    }
    mutex.unlock();
    interface->updateFund(uniqueId, money);
    interface->updateStock(uniqueId, &stocks);

    //Temps simulant l'assemblage d'un objet.
    PcoThread::usleep((rand() % 100) * 100000);

    mutex.lock();
    ++stocks[itemBuilt];
    ++nbBuild;
    mutex.unlock();

    interface->consoleAppendText(uniqueId, "Factory have build a new object");
}

void Factory::orderResources() {

    // TODO - Itérer sur les resourcesNeeded et les wholesalers disponibles
    for (ItemType item : resourcesNeeded) {
        if(stocks[item] != 0) {
            break;
        }
        int qty = 1;
        int price = qty * getCostPerUnit(item);
        if (money < price) { /**TODO: Il faut continue ou break? */
            /* Pas assez d'argent */
            /* Attend des jours meilleurs */
            PcoThread::usleep(1000U);
            continue;
        }
        for (auto wholesaler: wholesalers) {
            std::map<ItemType, int> itemsForSale = wholesaler->getItemsForSale();
            if (itemsForSale.find(item) != itemsForSale.end()) {
                interface->consoleAppendText(uniqueId, QString("Factory would like to buy %1 of ").arg(qty) %
                                             getItemName(item) % QString(" which would cost me %1").arg(price));
                if (wholesaler->trade(item, qty) == price) {
                    mutex.lock();
                    stocks[item] += qty;
                    money -= price;
                    mutex.unlock();
                    interface->updateFund(uniqueId, money);
                    interface->updateStock(uniqueId, &stocks);
                    interface->consoleAppendText(uniqueId, QString("Factory bought %1 of ").arg(qty) %
                                                 getItemName(item) % QString(" for %1").arg(price));
                }
            }
        }
    }

    //Temps de pause pour éviter trop de demande
    PcoThread::usleep(10 * 100000);

}

void Factory::run() {
    if (wholesalers.empty()) {
        std::cerr << "You have to give to factories wholesalers to sales their resources" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Factory routine");

    while (true /* TODO terminaison*/) {
        if (verifyResources()) {
            buildItem();
        } else {
            orderResources();
        }
        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
}

std::map<ItemType, int> Factory::getItemsForSale() {
    return std::map<ItemType, int>({{itemBuilt, stocks[itemBuilt]}});
}

int Factory::trade(ItemType it, int qty) {
    mutex.lock();
    if (conditionToTrade(it, qty)) {
        mutex.unlock();
        return 0;
    }
    stocks[it] -= qty;
    int cost = qty * getCostPerUnit(it);
    money += cost;
    mutex.unlock();
    interface->updateFund(uniqueId, money);
    interface->updateStock(uniqueId, &stocks);
    interface->consoleAppendText(uniqueId, QString("Factory sold %1 of ").arg(qty) %
                                 getItemName(it) % QString(" for %1").arg(cost));
    return cost;
}

bool Factory::conditionToTrade(ItemType it, int qty) {
    return qty <= 0
    || getItemsForSale().find(it) == getItemsForSale().end()
    || getItemsForSale().at(it) < qty;
}

int Factory::getAmountPaidToWorkers() {
    return Factory::nbBuild * getEmployeeSalary(getEmployeeThatProduces(itemBuilt));
}

void Factory::setInterface(WindowInterface *windowInterface) {
    interface = windowInterface;
}

PlasticFactory::PlasticFactory(int uniqueId, int fund) :
    Factory::Factory(uniqueId, fund, ItemType::Plastic, {ItemType::Petrol}) {}

ChipFactory::ChipFactory(int uniqueId, int fund) :
    Factory::Factory(uniqueId, fund, ItemType::Chip, {ItemType::Sand, ItemType::Copper}) {}

RobotFactory::RobotFactory(int uniqueId, int fund) :
    Factory::Factory(uniqueId, fund, ItemType::Robot, {ItemType::Chip, ItemType::Plastic}) {}
