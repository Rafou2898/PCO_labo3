#include "wholesale.h"
#include "factory.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

WindowInterface* Wholesale::interface = nullptr;

Wholesale::Wholesale(int uniqueId, int fund)
    : Seller(fund, uniqueId)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Wholesaler Created");

}

void Wholesale::setSellers(std::vector<Seller*> sellers) {
    this->sellers = sellers;

    for(Seller* seller: sellers){
        interface->setLink(uniqueId, seller->getUniqueId());
    }
}

void Wholesale::buyResources() {
    auto s = Seller::chooseRandomSeller(sellers);
    auto m = s->getItemsForSale();
    auto i = Seller::chooseRandomItem(m);

    if (i == ItemType::Nothing) {
        /* Nothing to buy... */
        return;
    }

    int qty = rand() % 5 + 1;
    int price = qty * getCostPerUnit(i);

    interface->consoleAppendText(uniqueId, QString("Wholesaler would like to buy %1 of ").arg(qty) %
                                 getItemName(i) % QString(" which would cost me %1").arg(price));
    if (price <= money && (s->trade(i, qty) == price)) {
        mutex.lock();
        stocks[i] += qty;
        money -= price;
        mutex.unlock();
        interface->consoleAppendText(uniqueId, QString("Wholesaler bought %1 of ").arg(qty) %
                                     getItemName(i) % QString(" for %1").arg(price));
    }
}

void Wholesale::run() {


    if (sellers.empty()) {
        std::cerr << "You have to give factories and mines to a wholeseler before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Wholesaler routine");
    while (!PcoThread::thisThread()->stopRequested() /* TODO terminaison*/) {

        buyResources();
        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        //Temps de pause pour espacer les demandes de ressources
        PcoThread::usleep((rand() % 10 + 1) * 100000);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Wholesaler routine");


}

std::map<ItemType, int> Wholesale::getItemsForSale() {
    return stocks;
}


int Wholesale::trade(ItemType it, int qty) {
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
    interface->consoleAppendText(uniqueId, QString("Wholesaler sold %1 of ").arg(getUniqueId(), qty) %
                                 getItemName(it) % QString(" for %1").arg(cost));
    return cost;
}

bool Wholesale::conditionToTrade(ItemType it, int qty) {
    return qty <= 0 || stocks[it] < qty;
}

void Wholesale::setInterface(WindowInterface *windowInterface) {
    interface = windowInterface;
}
